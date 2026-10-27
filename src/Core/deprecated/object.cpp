#include <Core/archive.hpp>
#include <Core/blob.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/compressor.hpp>
#include <Core/constants.hpp>
#include <Core/deprecated/object.hpp>
#include <Core/etl/templates.hpp>
#include <Core/file_flag.hpp>
#include <Core/filesystem/file.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/memory.hpp>
#include <Core/object_listener.hpp>
#include <Core/package.hpp>
#include <Core/pointer.hpp>
#include <Core/reflection/class.hpp>
#include <Core/stream.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Engine/project.hpp>
#include <ScriptEngine/script_binding.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Trinex
{
	static ScriptFunction script_object_preload;
	static ScriptFunction script_object_postload;

	static thread_local struct NextObjectInfo
	{
		Refl::Class* class_instance;

		NextObjectInfo() { reset(); }

		void reset() { class_instance = nullptr; }

	} s_next_object_info;

	static Package* s_root_package = nullptr;

	static Array<DeprecatedObject*> s_root_objects;
	static Array<DeprecatedObject*> s_objects;

	static void create_default_package()
	{
		if (s_root_package == nullptr)
		{
			s_root_package = DeprecatedObject::new_instance<Package>("Content");
			s_root_package->flags.remove(DeprecatedObject::Flags::IsSerializable);
			s_root_package->add_reference();
		}
	}

	Refl::Class* DeprecatedObject::static_setup_next_object_info(Refl::Class* self)
	{
		if (!s_next_object_info.class_instance)
		{
			s_next_object_info.class_instance = self;
			return self;
		}

		return s_next_object_info.class_instance;
	}

	void DeprecatedObject::initialize_new_object(DeprecatedObject* object, StringView name, DeprecatedObject* owner)
	{
		object->m_name = name;
		object->owner(owner);
		object->on_create();

		ObjectCreateListener::for_each_invoke(object);
		s_next_object_info.reset();
	}

	trinex_implement_engine_class(DeprecatedObject, Refl::Class::IsScriptable)
	{
		auto r = ScriptBinding::Class::existing(static_reflection());
		r.method("const string& string_name() const final", &DeprecatedObject::string_name);
		r.static_function("Package@ root_package()", &DeprecatedObject::root_package);
		r.method("const Name& name() const final", overload_of<const Name&>(&DeprecatedObject::name));
		r.method("Refl::Class@ class_instance() const final", &DeprecatedObject::class_instance);
		r.method("bool owner(DeprecatedObject@ new_owner) final", overload_of<bool()>(&DeprecatedObject::owner));

		r.method("void preload()", trinex_scoped_method(DeprecatedObject, preload));
		r.method("void postload()", trinex_scoped_method(DeprecatedObject, postload));
		script_object_preload  = r.type_info().method_by_decl("void preload()");
		script_object_postload = r.type_info().method_by_decl("void postload()");

		ScriptEngine::on_terminate.push([]() {
			script_object_preload.release();
			script_object_postload.release();
		});

		{
			ScriptNamespaceScopedChanger changer("Trinex::DeprecatedObject");
			ScriptEngine::register_function("DeprecatedObject@ static_find_object(StringView object_name)", static_find_object);
		}
	}

	void DeprecatedObject::script_preload()
	{
		ScriptContext::current()->execute(this, script_object_preload);
	}

	void DeprecatedObject::script_postload()
	{
		ScriptContext::current()->execute(this, script_object_postload);
	}

	bool DeprecatedObject::private_check_instance(const Refl::Class* const check_class) const
	{
		const void* self = this;
		if (self == nullptr)
			return false;

		auto _class = class_instance();
		return _class != nullptr && _class->is_a(check_class);
	}

	int DeprecatedObject::AddRef() const
	{
		return 1;
	}

	int DeprecatedObject::Release() const
	{
		return 1;
	}

	asILockableSharedBool* DeprecatedObject::GetWeakRefFlag() const
	{
		return nullptr;
	}

	int DeprecatedObject::GetRefCount()
	{
		return 1;
	}

	void DeprecatedObject::SetGCFlag() {}

	bool DeprecatedObject::GetGCFlag()
	{
		return false;
	}

	asITypeInfo* DeprecatedObject::GetObjectType() const
	{
		return m_class->script_type_info.info();
	}

	int DeprecatedObject::CopyFrom(const asIScriptObject* other)
	{
		return 0;
	}

	DeprecatedObject::DeprecatedObject() : m_references(0)
	{
		trinex_verify_msg(s_next_object_info.class_instance, "Next object class is invalid!");

		flags.set(Flags::IsSerializable | Flags::IsEditable | Flags::IsAvailableForGC);

		m_owner = nullptr;
		m_class = s_next_object_info.class_instance;
		s_next_object_info.reset();

		m_child_index = s_root_objects.size();
		s_root_objects.push_back(this);

		m_global_index = s_objects.size();
		s_objects.push_back(this);
	}

	class Refl::Class* DeprecatedObject::class_instance() const
	{
		return m_class;
	}

	trinex_api String DeprecatedObject::package_name_of(const StringView& name)
	{
		return String(package_name_sv_of(name));
	}

	trinex_api String DeprecatedObject::object_name_of(const StringView& name)
	{
		return String(object_name_sv_of(name));
	}

	trinex_api StringView DeprecatedObject::package_name_sv_of(const StringView& name)
	{
		auto index = name.find_last_of(Constants::name_separator);
		if (index == String::npos)
			return StringView();

		index -= 1;
		return name.substr(0, index);
	}

	trinex_api StringView DeprecatedObject::object_name_sv_of(const StringView& name)
	{
		auto pos = name.find_last_of(Constants::name_separator);
		if (pos == String::npos)
		{
			return name;
		}

		pos += Constants::name_separator.length() - 1;
		return name.substr(pos, name.length() - pos);
	}

	trinex_api const Array<DeprecatedObject*>& DeprecatedObject::static_root_objects()
	{
		return s_root_objects;
	}

	const Array<DeprecatedObject*>& DeprecatedObject::static_objects()
	{
		return s_objects;
	}

	DeprecatedObject::~DeprecatedObject()
	{
		remove_from<&DeprecatedObject::m_global_index>(s_objects, m_global_index);
	}

	const String& DeprecatedObject::string_name() const
	{
		return m_name.to_string();
	}

	DeprecatedObject* DeprecatedObject::find_child_object(StringView name) const
	{
		return nullptr;
	}

	bool DeprecatedObject::rename(StringView name, DeprecatedObject* new_owner)
	{
		if (new_owner == nullptr)
			new_owner = m_owner;

		if (name == m_name && new_owner == m_owner)
			return true;

		if (!name.empty())
		{
			String validation;
			if (!static_validate_object_name(name, &validation))
			{
				trinex_error(Log::Core, "%s", validation.c_str());
				return false;
			}
		}

		DeprecatedObject* old_owner = m_owner;
		Name old_name     = m_name;

		bool result = owner(nullptr);

		if (result)
		{
			m_name = name;
		}

		if (new_owner)
		{
			result = owner(new_owner);

			if (!result)
			{
				m_name = old_name;
				owner(old_owner);
			}
		}

		if (result)
		{
			post_rename(old_owner, old_name);
		}

		return result;
	}

	DeprecatedObject& DeprecatedObject::post_rename(DeprecatedObject* old_owner, Name old_name)
	{
		return *this;
	}

	const Name& DeprecatedObject::name() const
	{
		return m_name;
	}

	DeprecatedObject& DeprecatedObject::on_owner_update(DeprecatedObject* new_owner)
	{
		return *this;
	}

	DeprecatedObject* DeprecatedObject::register_child(DeprecatedObject* child, u32& index)
	{
		return this;
	}

	bool DeprecatedObject::unregister_child(DeprecatedObject* child)
	{
		return true;
	}

	Package* DeprecatedObject::package(bool recursive) const
	{
		if (recursive)
		{
			const DeprecatedObject* self = this;
			Package* pkg       = nullptr;

			while (self && !pkg)
			{
				pkg  = instance_cast<Package>(self->m_owner);
				self = self->m_owner;
			}

			return pkg;
		}

		return instance_cast<Package>(m_owner);
	}

	String DeprecatedObject::full_name() const
	{
		static auto object_name_of = [](const DeprecatedObject* object) -> String {
			if (object->m_name.is_valid())
				return object->m_name.to_string();

			return Strings::format("Noname object {}", object->m_global_index);
		};

		String result         = object_name_of(this);
		const DeprecatedObject* current = m_owner;
		const DeprecatedObject* root    = root_package();

		while (current && current != root)
		{
			String current_name = (current->m_name.is_valid() ? current->m_name.to_string()
			                                                  : Strings::format("Noname object {}", current->m_global_index));

			result  = Strings::format("{}{}{}", current_name, Constants::name_separator, result);
			current = current->m_owner;
		}

		return result;
	}

	u32 DeprecatedObject::references() const
	{
		return m_references;
	}

	u32 DeprecatedObject::add_reference() const
	{
		++m_references;
		return references();
	}

	u32 DeprecatedObject::remove_reference() const
	{
		--m_references;
		return references();
	}

	bool DeprecatedObject::is_noname() const
	{
		return !m_name.is_valid();
	}

	trinex_api const Package* root_package()
	{
		return s_root_package;
	}

	trinex_api DeprecatedObject* DeprecatedObject::static_find_object(StringView object_name)
	{
		return s_root_package->find_child_object(object_name);
	}

	DeprecatedObject& DeprecatedObject::preload()
	{
		return *this;
	}

	DeprecatedObject& DeprecatedObject::postload()
	{
		return rebuild();
	}

	DeprecatedObject& DeprecatedObject::rebuild()
	{
		return *this;
	}

	DeprecatedObject& DeprecatedObject::on_create()
	{
		return *this;
	}

	DeprecatedObject& DeprecatedObject::on_destroy()
	{
		if (m_owner)
		{
			m_owner->unregister_child(this);
			m_owner = nullptr;
		}
		else
		{
			remove_from(s_root_objects);
		}
		return *this;
	}

	DeprecatedObject& DeprecatedObject::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		return *this;
	}

	DeprecatedObject* DeprecatedObject::owner() const
	{
		return m_owner;
	}

	bool DeprecatedObject::owner(DeprecatedObject* new_owner)
	{
		if (new_owner == m_owner)
			return true;

		if (this == new_owner)
		{
			trinex_error(Log::Core, "DeprecatedObject cannot own itself!");
			return false;
		}

		const bool is_global = m_owner == nullptr;

		if (m_owner)
		{
			if (!m_owner->unregister_child(this))
			{
				trinex_error(Log::Core, "Failed to unregister object from prev owner!");
				return false;
			}

			m_owner = nullptr;
		}

		const u32 idx = m_child_index;

		while (new_owner)
		{
			m_child_index    = 0xFFFFFFFF;
			m_owner          = new_owner;
			DeprecatedObject* redirect = new_owner->register_child(this, m_child_index);

			if (redirect == new_owner)
			{
				if (is_global)
				{
					remove_from(s_root_objects, idx);
				}
				break;
			}
			else
			{
				new_owner = redirect;
			}
		}

		if (new_owner == nullptr)
		{
			if (is_global)
			{
				m_child_index = idx;
			}
			else
			{
				m_child_index = s_root_objects.size();
				s_root_objects.push_back(this);
			}
		}

		on_owner_update(m_owner);
		return true;
	}

	static inline Package* find_next_package(Package* package, StringView name, bool create)
	{
		if (name.empty())
			return package;

		Package* next_package = package->find_child_object_checked<Package>(name);
		if (next_package == nullptr && create && !name.empty())
		{
			next_package = DeprecatedObject::new_instance<Package>(name, package);
		}
		return next_package;
	}

	Package* DeprecatedObject::static_find_package(StringView name, bool create)
	{
		Package* package = const_cast<Package*>(root_package());

		const String& separator_text = Constants::name_separator;
		const usize separator_len    = separator_text.length();

		usize separator = name.find(separator_text);

		while (separator != StringView::npos && package)
		{
			package   = find_next_package(package, name.substr(0, separator), create);
			name      = name.substr(separator + separator_len);
			separator = name.find(separator_text);
		}

		return package ? find_next_package(package, name, create) : nullptr;
	}

	u32 DeprecatedObject::child_index() const
	{
		return m_child_index;
	}

	u32 DeprecatedObject::global_index() const
	{
		return m_global_index;
	}

	bool DeprecatedObject::serialize(Archive& archive)
	{
		if (!flags.any(Flags::IsSerializable))
		{
			return false;
		}

		return class_instance()->serialize_properties(this, archive);
	}

	bool DeprecatedObject::is_valid() const
	{
		return true;
	}

	Path DeprecatedObject::filepath() const
	{
		Path path = m_name.to_string() + Constants::asset_extention;

		const DeprecatedObject* entry = m_owner;
		const DeprecatedObject* root  = root_package();

		while (entry && entry != root)
		{
			path  = Path(entry->string_name()) / path;
			entry = entry->m_owner;
		}

		return path;
	}

	bool DeprecatedObject::is_editable() const
	{
		return flags.any(Flags::IsEditable);
	}

	const DeprecatedObject& DeprecatedObject::mark_dirty() const
	{
		flags |= Flags::IsDirty;
		return *this;
	}

	bool DeprecatedObject::is_dirty() const
	{
		return flags.all(Flags::IsDirty);
	}

	bool DeprecatedObject::save(Stream* stream, ArchiveFlags flags)
	{
		if (stream == nullptr)
		{
			Path path = Path(Project::assets_dir) / filepath();

			if (auto stream = rootfs()->open(path, VFS::AccessFlags::Write | VFS::AccessFlags::Recursive))
			{
				return save(stream.value(), flags);
			}
			else
			{
				trinex_error(Log::Core, "Failed to save object'%s': Failed to create file '%s'!", full_name().c_str(),
				             path.c_str());

				return false;
			}
		}

		if (!flags.any(Flags::IsSerializable))
		{
			trinex_error(Log::Core, "Cannot save non-serializable package!");
			return false;
		}

		Buffer buffer;
		Archive raw = Archive(buffer, IOMode::Write, flags);

		{
			DeprecatedObject* self = this;
			if (!raw.serialize_object(self))
			{
				return false;
			}
		}

		Array<u8> compressed;
		Compressor::compress(buffer, compressed);

		Archive ar(retain_ref(stream), IOMode::Write);
		FileFlag flag = FileFlag::asset_flag();
		ar.serialize(flag);
		ar.serialize(compressed);

		return ar;
	}

	trinex_api DeprecatedObject* DeprecatedObject::load_object(StringView fullname, Stream* stream, ArchiveFlags serialization_flags)
	{
		if (stream == nullptr)
		{
			trinex_error(Log::Core, "Cannot load object from nullptr buffer!");
			return nullptr;
		}

		if (!(serialization_flags & ArchiveFlags::SkipObjectSearch))
		{
			if (DeprecatedObject* object = static_find_object(fullname))
			{
				return object;
			}
		}

		Archive ar(retain_ref(stream), IOMode::Read);
		FileFlag flag = FileFlag::asset_flag();
		ar.serialize(flag);
		if (flag != FileFlag::asset_flag())
		{
			trinex_error(Log::Core, "Cannot load object. Asset flag mismatch!");
			return nullptr;
		}

		Array<u8> compressed_buffer;
		if (!(ar.serialize(compressed_buffer)))
		{
			trinex_error(Log::Core, "Failed to read compressed buffer!");
			return nullptr;
		}

		Array<u8> raw_data;
		Compressor::decompress(compressed_buffer, raw_data);
		BufferStream raw_stream = raw_data;

		Archive raw = Archive(retain_ref(&raw_stream), IOMode::Read);

		DeprecatedObject* object = nullptr;

		if (fullname.empty())
		{
			raw.serialize_object(object);
		}
		else
		{
			StringView package_name = DeprecatedObject::package_name_sv_of(fullname);
			StringView object_name  = DeprecatedObject::object_name_sv_of(fullname);

			raw.serialize_object(object, object_name, Package::static_find_package(package_name, true));
		}

		return object;
	}

	static DeprecatedObject* load_from_file_internal(const Path& path, StringView fullname, ArchiveFlags flags)
	{
		if (auto file = rootfs()->open(path))
		{
			DeprecatedObject* object = DeprecatedObject::load_object(fullname, file.value(), flags);
			return object;
		}

		return nullptr;
	}

	trinex_api DeprecatedObject* DeprecatedObject::load_object(StringView name, ArchiveFlags flags)
	{
		if (!(flags & ArchiveFlags::SkipObjectSearch))
		{
			if (DeprecatedObject* object = static_find_object(name))
				return object;
		}

		Path path =
		        Path(Project::assets_dir)
		        / Path(Strings::replace_all(name, Constants::name_separator, Path::sv_separator) + Constants::asset_extention);
		return load_from_file_internal(path, name, flags | ArchiveFlags::SkipObjectSearch);
	}

	trinex_api DeprecatedObject* DeprecatedObject::load_object_from_file(const Path& path, ArchiveFlags flags)
	{
		String package_name = Strings::replace_all(path.parent(), Path::sv_separator, Constants::name_separator);
		StringView name     = path.stem();
		String full_name    = package_name + Constants::name_separator + String(name);

		if (!(flags & ArchiveFlags::SkipObjectSearch))
		{
			if (DeprecatedObject* object = static_find_object(full_name))
				return object;
		}

		return load_from_file_internal(Path(Project::assets_dir) / path, full_name, flags | ArchiveFlags::SkipObjectSearch);
	}

	bool DeprecatedObject::is_serializable() const
	{
		return flags.any(Flags::IsSerializable);
	}

	Package* DeprecatedObject::root_package()
	{
		create_default_package();
		return s_root_package;
	}

	bool DeprecatedObject::static_validate_object_name(StringView name, String* msg)
	{
		auto create_msg = [msg](const char* message) {
			if (msg)
				*msg = message;
		};

		if (name.empty())
		{
			create_msg("The object name can't be empty!");
			return false;
		}

		auto non_supported_symbol = name.find_first_of("/\\:*?\"<>|, ");
		if (non_supported_symbol != StringView::npos)
		{
			create_msg(Strings::format("The object name cannot contain the character '{}'", name[non_supported_symbol]).c_str());
			return false;
		}

		return true;
	}

	trinex_api DeprecatedObject* DeprecatedObject::copy_from(DeprecatedObject* src)
	{
		ArchiveFlags flags = ArchiveFlags::IsCopyProcess;

		Buffer buffer;
		{
			BufferStream stream = buffer;

			if (!src->save(&stream, flags))
			{
				trinex_error(Log::Core, "Failed to save object to buffer!");
				return nullptr;
			}
		}

		{
			BufferStream stream = buffer;
			DeprecatedObject* new_object  = DeprecatedObject::load_object("", &stream, flags);

			if (!new_object)
			{
				trinex_error(Log::Core, "Failed to create copy of object!");
			}

			return new_object;
		}
	}

	DeprecatedObject* DeprecatedObject::static_new_instance(Refl::Class* object_class, StringView name, DeprecatedObject* owner)
	{
		if (object_class)
		{
			return object_class->create_object(name, owner);
		}
		return nullptr;
	}

	DeprecatedObject* DeprecatedObject::static_new_placement_instance(void* place, Refl::Class* object_class, StringView name, DeprecatedObject* owner)
	{
		if (object_class)
		{
			return object_class->create_placement_object(place, name, owner);
		}
		return nullptr;
	}
} // namespace Trinex
