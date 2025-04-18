#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/compressor.hpp>
#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/file_flag.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/pointer.hpp>
#include <Core/reflection/class.hpp>
#include <Core/render_resource.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Engine/project.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>
#include <angelscript.h>

namespace Engine
{
	static ScriptFunction script_object_preload;
	static ScriptFunction script_object_postload;

	static thread_local struct NextObjectInfo {
		Refl::Class* class_instance;

		NextObjectInfo() { reset(); }

		void reset() { class_instance = nullptr; }

	} next_object_info;

	Refl::Class* Object::static_setup_next_object_info(Refl::Class* self)
	{
		if (!next_object_info.class_instance)
		{
			next_object_info.class_instance = self;
			return self;
		}

		return next_object_info.class_instance;
	}

	void Object::static_setup_new_object(Object* object, StringView name, Object* owner)
	{
		object->m_name = name;
		object->owner(owner);
		object->on_create();
		next_object_info.reset();
	}

	trinex_implement_engine_class(Object, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_class_instance());
		r.method("const string& string_name() const final", &Object::string_name);
		r.static_function("Package@ root_package()", &Object::root_package);
		r.method("const Name& name() const final", method_of<const Name&>(&Object::name));
		r.method("Refl::Class@ class_instance() const final", &Object::class_instance);

		script_object_preload  = r.method("void preload()", trinex_scoped_method(Object, preload));
		script_object_postload = r.method("void postload()", trinex_scoped_method(Object, postload));

		ScriptEngine::on_terminate.push([]() {
			script_object_preload.release();
			script_object_postload.release();
		});

		{
			ScriptNamespaceScopedChanger changer("Engine::Object");
			ScriptEngine::register_function("Object@ static_find_object(StringView object_name)", static_find_object);
		}
	}

	static Vector<Index>& get_free_indexes_array()
	{
		static Vector<Index> array;
		return array;
	}

	static Vector<Object*>& get_instances_array()
	{
		static Vector<Object*> array;
		return array;
	}

	static Package* s_root_package = nullptr;

	static void create_default_package()
	{
		if (s_root_package == nullptr)
		{
			s_root_package = Object::new_instance<Package>("Content");
			s_root_package->flags(Object::IsSerializable, false);
			s_root_package->add_reference();
		}
	}

	void Object::script_preload()
	{
		ScriptObject(this).execute(script_object_preload);
	}

	void Object::script_postload()
	{
		ScriptObject(this).execute(script_object_postload);
	}

	bool Object::private_check_instance(const Refl::Class* const check_class) const
	{
		const void* self = this;
		if (self == nullptr)
			return false;

		auto _class = class_instance();
		return _class != nullptr && _class->is_a(check_class);
	}

	int Object::AddRef() const
	{
		return 1;
	}

	int Object::Release() const
	{
		return 1;
	}

	asILockableSharedBool* Object::GetWeakRefFlag() const
	{
		return nullptr;
	}

	int Object::GetRefCount()
	{
		return 1;
	}

	void Object::SetGCFlag() {}

	bool Object::GetGCFlag()
	{
		return false;
	}

	asITypeInfo* Object::GetObjectType() const
	{
		return m_class->script_type_info.info();
	}

	int Object::CopyFrom(const asIScriptObject* other)
	{
		return 0;
	}

	Object::Object() : m_references(0), m_instance_index(Constants::index_none)
	{
		trinex_always_check(is_in_logic_thread(), "Cannot create new object instance outside logic thread!");
		if (next_object_info.class_instance == nullptr)
		{
			throw EngineException("Next object class is invalid!");
		}

		flags(Flag::IsSerializable, true);
		flags(Flag::IsEditable, true);
		flags(Flag::IsAvailableForGC, true);

		m_owner = nullptr;
		m_class = next_object_info.class_instance;
		next_object_info.reset();


		Vector<Object*>& objects_array = get_instances_array();

		if (!get_free_indexes_array().empty())
		{
			m_instance_index = get_free_indexes_array().back();
			get_free_indexes_array().pop_back();
			objects_array[m_instance_index] = this;
		}
		else
		{
			m_instance_index = objects_array.size();
			objects_array.push_back(this);
		}
	}

	class Refl::Class* Object::class_instance() const
	{
		return m_class;
	}

	ENGINE_EXPORT HashIndex Object::hash_of_name(const StringView& name)
	{
		return memory_hash_fast(name.data(), name.length(), 0);
	}

	HashIndex Object::hash_index() const
	{
		return m_name.hash();
	}

	ENGINE_EXPORT String Object::package_name_of(const StringView& name)
	{
		return String(package_name_sv_of(name));
	}

	ENGINE_EXPORT String Object::object_name_of(const StringView& name)
	{
		return String(object_name_sv_of(name));
	}

	ENGINE_EXPORT StringView Object::package_name_sv_of(const StringView& name)
	{
		auto index = name.find_last_of(Constants::name_separator);
		if (index == String::npos)
			return StringView();

		index -= 1;
		return name.substr(0, index);
	}

	ENGINE_EXPORT StringView Object::object_name_sv_of(const StringView& name)
	{
		auto pos = name.find_last_of(Constants::name_separator);
		if (pos == String::npos)
		{
			return name;
		}

		pos += Constants::name_separator.length() - 1;
		return name.substr(pos, name.length() - pos);
	}


	const Object& Object::remove_from_instances_array() const
	{
		if (engine_instance && m_instance_index < get_instances_array().size())
		{
			get_instances_array()[m_instance_index] = nullptr;
			if (!engine_instance->is_shuting_down())
			{
				get_free_indexes_array().push_back(m_instance_index);
			}
			m_instance_index = Constants::max_size;
		}
		return *this;
	}

	ENGINE_EXPORT const Vector<Object*>& Object::all_objects()
	{
		return get_instances_array();
	}

	Object::~Object()
	{
		if (m_owner)
		{
			m_owner->unregister_child(this);
			m_owner = nullptr;
		}

		remove_from_instances_array();
	}

	const String& Object::string_name() const
	{
		return m_name.to_string();
	}

	Object* Object::find_child_object(StringView name) const
	{
		return nullptr;
	}

	bool Object::rename(StringView name, Object* new_owner)
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
				error_log("Object", "%s", validation.c_str());
				return false;
			}
		}

		Object* old_owner = m_owner;
		Name old_name     = m_name;

		bool result = true;

		if (new_owner != m_owner)
		{
			result = owner(nullptr);
		}

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

	Object& Object::post_rename(Object* old_owner, Name old_name)
	{
		return *this;
	}

	const Name& Object::name() const
	{
		return m_name;
	}

	Object* Object::noname_object()
	{
		static thread_local byte data[sizeof(Object)];
		return reinterpret_cast<Object*>(data);
	}

	Object& Object::on_owner_update(Object* new_owner)
	{
		return *this;
	}

	bool Object::register_child(Object* child)
	{
		return true;
	}

	bool Object::unregister_child(Object* child)
	{
		return true;
	}

	Package* Object::package(bool recursive) const
	{
		if (recursive)
		{
			const Object* self = this;
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

	String Object::full_name() const
	{
		static auto object_name_of = [](const Object* object) -> String {
			if (object->m_name.is_valid())
				return object->m_name.to_string();

			return Strings::format("Noname object {}", object->m_instance_index);
		};

		String result         = object_name_of(this);
		const Object* current = m_owner;
		const Object* root    = root_package();

		while (current && current != root)
		{
			String current_name = (current->m_name.is_valid() ? current->m_name.to_string()
			                                                  : Strings::format("Noname object {}", current->m_instance_index));

			result  = Strings::format("{}{}{}", current_name, Constants::name_separator, result);
			current = current->m_owner;
		}

		return result;
	}

	Counter Object::references() const
	{
		return m_references;
	}

	size_t Object::add_reference() const
	{
		++m_references;
		return references();
	}

	size_t Object::remove_reference() const
	{
		--m_references;
		return references();
	}

	bool Object::is_noname() const
	{
		return !m_name.is_valid();
	}

	ENGINE_EXPORT const Package* root_package()
	{
		return s_root_package;
	}

	ENGINE_EXPORT Object* Object::static_find_object(StringView object_name)
	{
		return s_root_package->find_child_object(object_name);
	}

	Object& Object::preload()
	{
		return *this;
	}

	Object& Object::postload()
	{
		return *this;
	}

	Object& Object::apply_changes()
	{
		return *this;
	}

	Object& Object::on_create()
	{
		return *this;
	}

	Object& Object::on_destroy()
	{
		return *this;
	}

	Object& Object::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		return *this;
	}

	Object* Object::owner() const
	{
		return m_owner;
	}

	bool Object::owner(Object* new_owner)
	{
		if (new_owner == m_owner)
			return true;

		bool result = true;

		if (m_owner)
		{
			result  = m_owner->unregister_child(this);
			m_owner = nullptr;

			if (!result)
			{
				error_log("Object", "Failed to unregister object from prev owner!");
				return false;
			}
		}

		if (new_owner)
		{
			if ((result = new_owner->register_child(this)))
			{
				m_owner = new_owner;
			}
			else
			{
				error_log("Object", "Failed to register object to owner!");
			}
		}

		on_owner_update(new_owner);
		return result;
	}

	static FORCE_INLINE Package* find_next_package(Package* package, const StringView& name, bool create)
	{
		Package* next_package = package->find_child_object_checked<Package>(name);
		if (next_package == nullptr && create && !name.empty())
		{
			next_package = Object::new_instance<Package>(name, package);
		}
		return next_package;
	}

	Package* Object::static_find_package(StringView name, bool create)
	{
		Package* package = const_cast<Package*>(root_package());

		const String& separator_text = Constants::name_separator;
		const size_t separator_len   = separator_text.length();

		size_t separator = name.find(separator_text);

		while (separator != StringView::npos && package)
		{
			package   = find_next_package(package, name.substr(0, separator), create);
			name      = name.substr(separator + separator_len);
			separator = name.find(separator_text);
		}

		return package ? find_next_package(package, name, create) : nullptr;
	}

	Index Object::instance_index() const
	{
		return m_instance_index;
	}

	bool Object::serialize(Archive& archive)
	{
		if (!flags(Flag::IsSerializable))
		{
			return false;
		}

		return class_instance()->serialize_properties(this, archive);
	}

	bool Object::is_valid() const
	{
		return true;
	}

	Path Object::filepath() const
	{
		Path path = m_name.to_string() + Constants::asset_extention;

		const Object* entry = m_owner;
		const Object* root  = root_package();

		while (entry && entry != root)
		{
			path  = Path(entry->string_name()) / path;
			entry = entry->m_owner;
		}

		return path;
	}

	bool Object::is_editable() const
	{
		return flags(IsEditable);
	}

	const Object& Object::mark_dirty() const
	{
		flags(IsDirty, true);
		return *this;
	}

	bool Object::is_dirty() const
	{
		return flags(IsDirty);
	}

	template<typename Type>
	static Type* open_asset_file(const Object* object, bool create_dir = false)
	{
		Path path = Path(Project::assets_dir) / object->filepath();

		if (create_dir)
		{
			rootfs()->create_dir(path.base_path());
		}

		Type* value = new Type(path);
		if (value->is_open())
		{
			return value;
		}

		delete value;

		if constexpr (std::is_base_of_v<BufferReader, Type>)
		{
			error_log("Object", "Failed to load object '%s': File '%s' not found!", object->full_name().c_str(), path.c_str());
		}
		else
		{
			error_log("Object", "Failed to save object'%s': Failed to create file '%s'!", object->full_name().c_str(),
			          path.c_str());
		}

		return nullptr;
	}

	bool Object::save(class BufferWriter* writer, SerializationFlags serialization_flags)
	{
		if (!flags(Object::IsSerializable))
		{
			error_log("Object", "Cannot save non-serializable package!");
			return false;
		}

		bool status;
		Vector<byte> raw_buffer;
		VectorWriter raw_writer = &raw_buffer;
		Archive raw_ar          = &raw_writer;
		raw_ar.flags            = serialization_flags;

		auto hierarchy = class_instance()->hierarchy(1);
		raw_ar.serialize(hierarchy);

		status = serialize(raw_ar);

		if (!status)
		{
			return false;
		}

		Vector<byte> compressed_buffer;
		Compressor::compress(raw_buffer, compressed_buffer);

		bool need_destroy_writer = (writer == nullptr);

		if (need_destroy_writer)
		{
			writer = open_asset_file<FileWriter>(this, true);
		}

		if (!writer)
			return false;

		Archive ar(writer);
		FileFlag flag = FileFlag::asset_flag();
		ar.serialize(flag);
		ar.serialize(compressed_buffer);

		if (need_destroy_writer)
		{
			delete writer;
		}

		return ar;
	}

	static FORCE_INLINE Refl::Class* find_class(const Vector<Name>& hierarchy)
	{
		Refl::Class* instance = nullptr;

		for (Name name : hierarchy)
		{
			if ((instance = Refl::Class::static_find(name)))
			{
				return instance;
			}
		}

		return instance;
	}

	ENGINE_EXPORT Object* Object::load_object(StringView fullname, class BufferReader* reader,
	                                          SerializationFlags serialization_flags)
	{
		if (reader == nullptr)
		{
			error_log("Object", "Cannot load object from nullptr buffer reader!");
			return nullptr;
		}

		if (!(serialization_flags & SerializationFlags::SkipObjectSearch))
		{
			if (Object* object = static_find_object(fullname))
			{
				return object;
			}
		}

		Archive ar(reader);
		FileFlag flag = FileFlag::asset_flag();
		ar.serialize(flag);
		if (flag != FileFlag::asset_flag())
		{
			error_log("Object", "Cannot load object. Asset flag mismatch!");
			return nullptr;
		}

		Vector<byte> compressed_buffer;
		if (!(ar.serialize(compressed_buffer)))
		{
			error_log("Object", "Failed to read compressed buffer!");
			return nullptr;
		}

		Vector<byte> raw_data;
		Compressor::decompress(compressed_buffer, raw_data);
		VectorReader raw_reader = &raw_data;
		Archive raw_ar          = &raw_reader;

		Vector<Name> hierarchy;
		raw_ar.serialize(hierarchy);

		Refl::Class* self = find_class(hierarchy);

		if (self == nullptr)
		{
			error_log("Object", "Cannot load object. Class '%s' not found!", hierarchy.front().c_str());
			return nullptr;
		}

		Object* object = self->create_object();

		if (!fullname.empty())
		{
			StringView package_name = package_name_sv_of(fullname);
			StringView object_name  = object_name_sv_of(fullname);

			object->rename(object_name, Package::static_find_package(package_name, true));
		}

		object->preload();
		bool valid = object->serialize(raw_ar);

		if (!valid)
		{
			error_log("Object", "Failed to load object");
			delete object;
			object = nullptr;
		}
		else
		{
			object->postload();
		}
		return object;
	}

	static Object* load_from_file_internal(const Path& path, StringView fullname, Flags<SerializationFlags> flags)
	{
		FileReader reader(path);
		if (reader.is_open())
		{
			Object* object = Object::load_object(fullname, &reader, flags);
			return object;
		}

		return nullptr;
	}

	ENGINE_EXPORT Object* Object::load_object(StringView name, SerializationFlags flags)
	{
		if (!(flags & SerializationFlags::SkipObjectSearch))
		{
			if (Object* object = static_find_object(name))
				return object;
		}

		Path path = Path(Project::assets_dir) /
		            Path(Strings::replace_all(name, Constants::name_separator, Path::sv_separator) + Constants::asset_extention);
		return load_from_file_internal(path, name, flags | SerializationFlags::SkipObjectSearch);
	}

	ENGINE_EXPORT Object* Object::load_object_from_file(const Path& path, SerializationFlags flags)
	{
		String package_name = Strings::replace_all(path.base_path(), Path::sv_separator, Constants::name_separator);
		StringView name     = path.stem();
		String full_name    = package_name + Constants::name_separator + String(name);

		if (!(flags & SerializationFlags::SkipObjectSearch))
		{
			if (Object* object = static_find_object(full_name))
				return object;
		}

		return load_from_file_internal(Path(Project::assets_dir) / path, full_name, flags | SerializationFlags::SkipObjectSearch);
	}

	bool Object::is_serializable() const
	{
		return flags(IsSerializable);
	}

	Package* Object::root_package()
	{
		create_default_package();
		return s_root_package;
	}

	bool Object::static_validate_object_name(StringView name, String* msg)
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

	ENGINE_EXPORT Object* Object::copy_from(Object* src)
	{
		Flags<SerializationFlags> flags = SerializationFlags::IsCopyProcess;

		Buffer buffer;
		{
			VectorWriter writer(&buffer);

			if (!src->save(&writer, flags))
			{
				error_log("Object", "Failed to save object to buffer!");
				return nullptr;
			}
		}

		{
			VectorReader reader(&buffer);
			Object* new_object = Object::load_object("", &reader, flags);

			if (!new_object)
			{
				error_log("Object", "Failed to create copy of object!");
			}

			return new_object;
		}
	}

	Object* Object::static_new_instance(Refl::Class* object_class, StringView name, Object* owner)
	{
		if (object_class)
		{
			return object_class->create_object(name, owner);
		}
		return nullptr;
	}

	Object* Object::static_new_placement_instance(void* place, Refl::Class* object_class, StringView name, Object* owner)
	{
		if (object_class)
		{
			return object_class->create_placement_object(place, name, owner);
		}
		return nullptr;
	}
}// namespace Engine
