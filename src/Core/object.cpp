#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/compressor.hpp>
#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/file_flag.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/render_resource.hpp>
#include <Core/string_functions.hpp>
#include <Engine/project.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_object.hpp>


namespace Engine
{

	static thread_local struct NextObjectInfo {
		Class* class_instance;

		NextObjectInfo()
		{
			reset();
		}

		void reset()
		{
			class_instance = nullptr;
		}

	} next_object_info;

	void Object::setup_next_object_info(Class* self, bool override)
	{
		if (next_object_info.class_instance && !override)
		{
			return;
		}

		next_object_info.class_instance = self;
	}

	void Object::reset_next_object_info()
	{
		next_object_info.reset();
	}

	static void register_object_to_script(ScriptClassRegistrar* registrar, Class* self)
	{
		registrar->method("const string& string_name() const", &Object::string_name);
		registrar->static_function("Package@ root_package()", &Object::root_package);
		registrar->method("string as_string() const", &Object::as_string);
		registrar->method("const Name& name() const", method_of<const Name&>(&Object::name));
		registrar->method("string opConv() const", &Object::as_string);
		registrar->method("Object@ preload()", func_of<Object&(Object*)>([](Object* self) -> Object& { return self->preload(); }),
		                  ScriptCallConv::CDeclObjFirst);
		registrar->method("Object@ postload()",
		                  func_of<Object&(Object*)>([](Object* self) -> Object& { return self->postload(); }),
		                  ScriptCallConv::CDeclObjFirst);
		registrar->method("Class@ class_instance() const",
		                  func_of<Class*(Object*)>([](Object* object) -> Class* { return object->class_instance(); }));
	}


	implement_engine_class(Object, Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = register_object_to_script;
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

	static Package* m_root_package = nullptr;

	void Object::create_default_package()
	{
		if (m_root_package == nullptr)
		{
			m_root_package = new_instance<Package>("Content");
			m_root_package->flags(IsSerializable, false);
			m_root_package->add_reference();
		}
	}

	bool Object::private_check_instance(const Class* const check_class) const
	{
		const void* self = this;
		if (self == nullptr)
			return false;

		auto _class = class_instance();
		return _class != nullptr && _class->is_a(check_class);
	}


	Object::Object() : m_references(0), m_instance_index(Constants::index_none)
	{
		if (next_object_info.class_instance == nullptr)
		{
			throw EngineException("Next object class is invalid!");
		}

		m_owner                    = nullptr;
		m_class                    = next_object_info.class_instance;
		ObjectArray& objects_array = get_instances_array();
		m_instance_index           = objects_array.size();

		if (!get_free_indexes_array().empty())
		{
			m_instance_index = get_free_indexes_array().back();
			get_free_indexes_array().pop_back();
			objects_array[m_instance_index] = this;
		}
		else
		{
			objects_array.push_back(this);
		}

		flags(Flag::IsSerializable, true);
		flags(Flag::IsEditable, true);
		flags(Flag::IsAvailableForGC, true);

		next_object_info.reset();
	}

	class Class* Object::class_instance() const
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

	ENGINE_EXPORT const ObjectArray& Object::all_objects()
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

	Object* Object::find_child_object(StringView name, bool recursive) const
	{
		return nullptr;
	}

	bool Object::rename(StringView name, Object* new_owner)
	{
		if (name.find(Constants::name_separator) != StringView::npos)
		{
			error_log("Object", "Failed to rename object. Name can't contains '%s'", Constants::name_separator.c_str());
			return false;
		}

		if (new_owner == nullptr)
			new_owner = m_owner;

		Object* old_owner = m_owner;
		Name old_name     = m_name;

		bool result = true;

		if (new_owner != m_owner)
		{
			result = owner(nullptr);
		}
		else if (m_owner)
		{
			result = m_owner->rename_child_object(this, name);
		}

		if (result)
		{
			m_name = name;
		}

		if (new_owner != m_owner)
		{
			result = owner(new_owner);
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

	bool Object::rename_child_object(Object* object, StringView new_name)
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
				pkg  = instance_cast<Package>(m_owner);
				self = m_owner;
			}

			return pkg;
		}

		return instance_cast<Package>(m_owner);
	}


	String Object::full_name(bool override_by_owner) const
	{
		static auto object_name_of = [](const Object* object) -> String {
			if (object->m_name.is_valid())
				return object->m_name.to_string();

			return Strings::format("Noname object {}", object->m_instance_index);
		};

		static auto parent_object_of = [](const Object* object, bool override_by_owner) -> Object* {
			if (override_by_owner)
			{
				Object* parent = object->owner();
				if (parent)
					return parent;
			}

			Object* parent = object->package();
			if (parent != Object::root_package())
				return parent;

			return nullptr;
		};

		String result         = object_name_of(this);
		const Object* current = parent_object_of(this, override_by_owner);

		while (current)
		{
			result =
			        Strings::format("{}{}{}",
			                        (current->m_name.is_valid() ? current->m_name.to_string()
			                                                    : Strings::format("Noname object {}", current->m_instance_index)),
			                        Constants::name_separator, result);
			current = parent_object_of(current, override_by_owner);
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
		return m_root_package;
	}

	ENGINE_EXPORT Object* Object::static_find_object(StringView object_name)
	{
		return m_root_package->find_child_object(object_name, true);
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
		Package* next_package = package->find_child_object_checked<Package>(name, false);
		if (next_package == nullptr && create)
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

	String Object::as_string() const
	{
		return Strings::format("{}: {}", class_instance()->name().c_str(),
		                       m_name.is_valid() ? m_name.to_string().c_str() : "NoName");
	}

	Index Object::instance_index() const
	{
		return m_instance_index;
	}

	bool Object::archive_process(Archive& archive)
	{
		if (!SerializableObject::archive_process(archive))
		{
			return false;
		}

		if (!flags(Flag::IsSerializable))
		{
			return false;
		}

		return serialize_object_properties(archive);
	}

	bool Object::is_valid() const
	{
		return true;
	}

	Path Object::filepath() const
	{
		Path path = m_name.to_string() + Constants::asset_extention;

		const Package* pkg  = package(true);
		const Package* root = root_package();

		while (pkg && pkg != root)
		{
			path = Path(pkg->string_name()) / path;
			pkg  = pkg->package();
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

	bool Object::save(class BufferWriter* writer, Flags<SerializationFlags> serialization_flags)
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
		raw_ar & hierarchy;

		status = archive_process(raw_ar);

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
		ar & flag;
		ar & compressed_buffer;

		if (need_destroy_writer)
		{
			delete writer;
		}

		return ar;
	}


	static FORCE_INLINE Class* find_class(const Vector<Name>& hierarchy)
	{
		Class* instance = nullptr;

		for (Name name : hierarchy)
		{
			if ((instance = Class::static_find(name, false)))
			{
				return instance;
			}
		}

		return instance;
	}

	ENGINE_EXPORT Object* Object::load_object(StringView fullname, class BufferReader* reader,
	                                          Flags<SerializationFlags> serialization_flags)
	{
		if (reader == nullptr)
		{
			error_log("Object", "Cannot load object from nullptr buffer reader!");
			return nullptr;
		}

		if (!serialization_flags(SerializationFlags::SkipObjectSearch))
		{
			if (Object* object = static_find_object(fullname))
			{
				return object;
			}
		}

		Archive ar(reader);
		FileFlag flag = FileFlag::asset_flag();
		ar & flag;
		if (flag != FileFlag::asset_flag())
		{
			error_log("Object", "Cannot load object. Asset flag mismatch!");
			return nullptr;
		}

		Vector<byte> compressed_buffer;
		if (!(ar & compressed_buffer))
		{
			error_log("Object", "Failed to read compressed buffer!");
			return nullptr;
		}

		Vector<byte> raw_data;
		Compressor::decompress(compressed_buffer, raw_data);
		VectorReader raw_reader = &raw_data;
		Archive raw_ar          = &raw_reader;

		Vector<Name> hierarchy;
		raw_ar & hierarchy;

		Class* self = find_class(hierarchy);

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
		bool valid = object->archive_process(raw_ar);

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

	ENGINE_EXPORT Object* Object::load_object(StringView name, Flags<SerializationFlags> flags)
	{
		if (!flags(SerializationFlags::SkipObjectSearch))
		{
			if (Object* object = static_find_object(name))
				return object;
		}

		Path path = Path(Project::assets_dir) /
		            Path(Strings::replace_all(name, Constants::name_separator, Path::sv_separator) + Constants::asset_extention);
		return load_from_file_internal(path, name, flags | SerializationFlags::SkipObjectSearch);
	}

	ENGINE_EXPORT Object* Object::load_object_from_file(const Path& path, Flags<SerializationFlags> flags)
	{
		String package_name = Strings::replace_all(path.base_path(), Path::sv_separator, Constants::name_separator);
		StringView name     = path.stem();
		String full_name    = package_name + Constants::name_separator + String(name);

		if (!flags(SerializationFlags::SkipObjectSearch))
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
		Object::create_default_package();
		return m_root_package;
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

	Object* Object::static_new_instance(Class* object_class, StringView name, Object* owner)
	{
		if (object_class)
		{
			return object_class->create_object(name, owner);
		}
		return nullptr;
	}

	Object* Object::static_new_placement_instance(void* place, Class* object_class, StringView name, Object* owner)
	{
		if (object_class)
		{
			return object_class->create_placement_object(place, name, owner);
		}
		return nullptr;
	}
}// namespace Engine
