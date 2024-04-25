#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/compressor.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
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
#include <ScriptEngine/script_object.hpp>
#include <angelscript.h>
#include <cstring>
#include <typeinfo>


namespace Engine
{

    static thread_local bool m_is_valid_next_object = false;

    implement_class(Object, Engine, Class::IsScriptable);

#define script_virtual_method(type, method)


    static FORCE_INLINE bool can_update_reference()
    {
        return engine_instance && !engine_instance->is_shuting_down();
    }

    static void add_object_reference(Object* object)
    {
        if (object && can_update_reference())
        {
            object->add_reference();
        }
    }

    static void remove_object_reference(Object* object)
    {
        if (object && can_update_reference())
        {
            object->remove_reference();
        }
    }

    static void register_object_to_script(ScriptClassRegistrar* registrar, Class* self)
    {
        String factory = Strings::format("{}@ f()", self->name().c_str());

        ScriptEngineInitializeController().require("Initialize bindings");

        if (!self->flags(Class::IsSingletone))
        {
            registrar->behave(ScriptClassBehave::Factory, factory.c_str(), self->static_constructor(), ScriptCallConv::CDECL);
        }

        registrar->require_type("Engine::Package");

        registrar->behave(ScriptClassBehave::AddRef, "void f()", add_object_reference, ScriptCallConv::CDECL_OBJFIRST)
                .behave(ScriptClassBehave::Release, "void f()", remove_object_reference, ScriptCallConv::CDECL_OBJFIRST)
                .method("const string& string_name() const", &Object::string_name)
                .method("Engine::ObjectRenameStatus name(StringView, bool = false)",
                        method_of<ObjectRenameStatus, Object, StringView, bool>(&Object::name))
                .static_function("Package@ root_package()", &Object::root_package)
                .method("string as_string() const", &Object::as_string)
                .method("bool add_to_package(Package@, bool)", &Object::add_to_package)
                .static_function("Package@ find_package(StringView, bool)",
                                 func_of<Package*(StringView, bool)>(&Object::find_package))
                .static_function("Object@ static_find_object(const StringView&)",
                                 func_of<Object*(const StringView&)>(&Object::find_object))
                .method("Object& remove_from_package()", &Object::remove_from_package)
                .method("const Name& name() const", method_of<const Name&, Object>(&Object::name))
                .method("string opConv() const", &Object::as_string)
                .virtual_method("Object@ preload()",
                                func_of<Object&(Object*)>([](Object* self) -> Object& { return self->preload(); }),
                                ScriptCallConv::CDECL_OBJFIRST)
                .virtual_method("Object@ postload()",
                                func_of<Object&(Object*)>([](Object* self) -> Object& { return self->postload(); }),
                                ScriptCallConv::CDECL_OBJFIRST)
                .virtual_method("Object@ destroy_script_object(?& in)",
                                func_of<Object&(Object * self, asIScriptObject * *obj)>(
                                        [](Object* self, asIScriptObject** obj) -> Object& {
                                            // Obj always must be descriptor!
                                            ScriptObject object = *obj;
                                            object.add_reference();
                                            return self->destroy_script_object(&object);
                                        }))
                .virtual_method("Engine::Class@ class_instance() const",
                                func_of<Class*(Object*)>([](Object* object) -> Class* { return object->class_instance(); }));
    }

    implement_initialize_class(Object)
    {
        ScriptEnumRegistrar("Engine::ObjectRenameStatus")
                .set("Skipped", static_cast<int_t>(ObjectRenameStatus::Skipped))
                .set("Success", static_cast<int_t>(ObjectRenameStatus::Success))
                .set("Failed", static_cast<int_t>(ObjectRenameStatus::Failed));

        static_class_instance()->set_script_registration_callback(register_object_to_script);
    }

    void Object::prepare_next_object_allocation()
    {
        m_is_valid_next_object = true;
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
            m_root_package = new_instance<Package>();
            m_root_package->name("Root Package");
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


    Object::Object() : m_package(nullptr), m_references(0), m_instance_index(Constants::index_none)
    {
        if (!m_is_valid_next_object)
        {
            throw EngineException("Use Object::new_instance method or operator new for allocating objects");
        }

        m_owner                    = nullptr;
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
        m_is_valid_next_object = false;
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
        if (m_package)
        {
            m_package->remove_object(this);
            m_package = nullptr;
        }
        remove_from_instances_array();
    }

    const String& Object::string_name() const
    {
        return m_name.to_string();
    }


    ObjectRenameStatus Object::name(StringView new_name, bool autorename)
    {
        if (m_name == new_name)
            return ObjectRenameStatus::Skipped;

        Package* package_backup = m_package;
        Name name_backup        = m_name;

        auto restore_object_name = [&package_backup, &name_backup, this]() {
            m_name = name_backup;
            if (package_backup)
            {
                package_backup->add_object(this);
            }
        };


        Package* package = m_package;
        if (package)
        {
            package->remove_object(this);
        }

        // Find package
        const String& separator_text = Constants::name_separator;
        size_t separator_index       = new_name.find_first_of(separator_text);

        if (separator_index != StringView::npos)
        {
            package = root_package();
        }

        while (separator_index != StringView::npos && package)
        {
            StringView package_name = new_name.substr(0, separator_index);
            Package* next_package   = package->find_object_checked<Package>(package_name, false);

            if (next_package == nullptr)
            {
                next_package = Object::new_instance<Package>();
                next_package->name(package_name);
                if (!package->add_object(next_package, false))
                {
                    restore_object_name();
                    return ObjectRenameStatus::Failed;
                }
            }

            package  = next_package;
            new_name = new_name.substr(separator_index + separator_text.length());

            separator_index = new_name.find(separator_text);
        }

        // Apply new object name
        m_name = new_name;

        if (package)
        {
            if (!package->add_object(this, autorename))
            {
                restore_object_name();
                return ObjectRenameStatus::Failed;
            }
        }

        return ObjectRenameStatus::Success;
    }


    const Name& Object::name() const
    {
        return m_name;
    }

    Object* Object::copy()
    {
        Object* object = nullptr;

        {
            object        = class_instance()->create_object();
            object->flags = flags;
        }

        return object;
    }

    bool Object::add_to_package(Package* package, bool autorename)
    {
        return package->add_object(this, autorename);
    }

    Object& Object::remove_from_package()
    {
        if (m_package && m_package != m_root_package)
            m_package->remove_object(this);
        return *this;
    }

    Object* Object::noname_object()
    {
        static thread_local byte data[sizeof(Object)];
        return reinterpret_cast<Object*>(data);
    }

    Package* Object::package() const
    {
        return m_package;
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

    void Object::add_reference()
    {
        ++m_references;
    }

    void Object::remove_reference()
    {
        --m_references;
    }

    bool Object::is_noname() const
    {
        return !m_name.is_valid();
    }


    ENGINE_EXPORT const Package* root_package()
    {
        return m_root_package;
    }

    ENGINE_EXPORT Object* Object::find_object(const StringView& object_name)
    {
        return m_root_package->find_object(object_name);
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

    Object& Object::owner(Object* new_owner)
    {
        m_owner = new_owner;
        return *this;
    }

    Object& Object::destroy_script_object(class ScriptObject* object)
    {
        return *this;
    }

    static FORCE_INLINE Package* find_next_package(Package* package, const StringView& name, bool create)
    {
        Package* next_package = package->find_object_checked<Package>(name, false);
        if (next_package == nullptr && create)
        {
            next_package = Object::new_instance<Package>();
            next_package->name(name);
            package->add_object(next_package);
        }
        return next_package;
    }

    Package* Object::find_package(StringView name, bool create)
    {
        Package* package = const_cast<Package*>(root_package());

        const String& separator_text = Constants::name_separator;
        const size_t separator_len   = separator_text.length();

        size_t separator = name.find_first_of(separator_text);

        while (separator != StringView::npos && package)
        {
            package   = find_next_package(package, name.substr(0, separator), create);
            name      = name.substr(separator + separator_len);
            separator = name.find_first_of(separator_text);
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
        if (flags(IsUnreachable))
            return false;
        return true;
    }

    Path Object::filepath() const
    {
        Path path = m_name.to_string() + Constants::asset_extention;

        const Package* pkg  = m_package;
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
        return flags(IsEditable) && !is_engine_resource();
    }

    bool Object::is_engine_resource() const
    {
        return false;
    }

    template<typename Type>
    static Type* open_asset_file(const Object* object, bool create_dir = false)
    {
        Path path = engine_config.assets_dir / object->filepath();

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

    ENGINE_EXPORT Object* Object::load_object(const StringView& fullname, class BufferReader* reader,
                                              Flags<SerializationFlags> serialization_flags)
    {
        if (reader == nullptr)
        {
            error_log("Object", "Cannot load object from nullptr buffer reader!");
            return nullptr;
        }

        if (!serialization_flags(SerializationFlags::SkipObjectSearch))
        {
            if (Object* object = find_object(fullname))
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

            object->name(object_name);

            if (!package_name.empty())
            {
                Package::find_package(package_name, true)->add_object(object);
            }
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

    ENGINE_EXPORT Object* Object::load_object(const StringView& name, Flags<SerializationFlags> flags)
    {
        if (!flags(SerializationFlags::SkipObjectSearch))
        {
            if (Object* object = find_object(name))
                return object;
        }

        Path path = engine_config.assets_dir /
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
            if (Object* object = find_object(full_name))
                return object;
        }

        return load_from_file_internal(engine_config.assets_dir / path, full_name, flags | SerializationFlags::SkipObjectSearch);
    }

    bool Object::is_serializable() const
    {
        return flags(IsSerializable) && !is_engine_resource();
    }

    Package* Object::root_package()
    {
        Object::create_default_package();
        return m_root_package;
    }
}// namespace Engine
