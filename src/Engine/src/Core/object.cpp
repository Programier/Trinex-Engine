#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/demangle.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/render_resource.hpp>
#include <Core/string_functions.hpp>
#include <cstring>
#include <typeinfo>


namespace Engine
{

    static thread_local bool _M_next_available_for_gc = false;

    implement_class(Object, "Engine", Class::IsScriptable);

#define script_virtual_method(type, method)

    static void register_object_to_script(ScriptClassRegistrar* registrar, Class* self)
    {
        String factory = Strings::format("{}@ f()", self->name());

        InitializeController().require("Bind class Name");

        if (!self->has_all_flags(Class::IsSingletone))
        {
            registrar->behave(ScriptClassBehave::Factory, factory.c_str(), self->static_constructor());
        }

        registrar->require_type("Engine::Package");

        registrar->behave(ScriptClassBehave::AddRef, "void f()", &Object::add_reference)
                .behave(ScriptClassBehave::Release, "void f()", &Object::remove_reference)
                .method("const string& string_name() const", &Object::string_name)
                .method("Engine::ObjectRenameStatus name(const string& in, bool = false)",
                        func_of<ObjectRenameStatus, Object, const String&, bool>(&Object::name))
                .method("Package@ root_package()", &Object::root_package)
                .method("ObjectRenameStatus name(string, bool) const",
                        method_of<ObjectRenameStatus, Object, const String&, bool>(&Object::name))
                .method("string as_string() const", &Object::as_string)
                .method("bool add_to_package(Package@, bool)", &Object::add_to_package)
                .method("Package@ find_package(const string& in, bool)",
                        func_of<Package*, const String&, bool>(&Object::find_package))
                .method("Object@ static_find_object(const string& in)", &Object::find_object)
                .method("Object& remove_from_package()", &Object::remove_from_package)
                .method("const Name& name() const", method_of<const Name&, Object>(&Object::name))
                .method("string opConv() const", &Object::as_string);
    }

    implement_initialize_class(Object)
    {
        ScriptEnumRegistrar("Engine::ObjectRenameStatus")
                .set("Skipped", static_cast<int_t>(ObjectRenameStatus::Skipped))
                .set("Success", static_cast<int_t>(ObjectRenameStatus::Success))
                .set("Failed", static_cast<int_t>(ObjectRenameStatus::Failed));

        static_class_instance()->set_script_registration_callback(register_object_to_script);
    }

    void Object::prepare_next_object_for_gc()
    {
        _M_next_available_for_gc = true;
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


    static Package* _M_root_package = nullptr;

    void Object::create_default_package()
    {
        if (_M_root_package == nullptr)
        {
            _M_root_package = new Package();
            _M_root_package->name("Root Package");
            _M_root_package->flag(IsSerializable, false);
        }
    }

    bool Object::object_is_exist(Package* package, const String& name)
    {
        if (package)
        {
            bool status = package->contains_object(name);
            if (status)
            {
                error_log("Object", "Cannot create new object. Object '%s' is exist in package '%s'!", name.c_str(),
                          package->full_name().c_str());
            }
            return status;
        }

        return false;
    }


    bool Object::private_check_instance(const Class* const check_class) const
    {
        auto _class = class_instance();
        return _class != nullptr && _class->contains_class(check_class);
    }


    Object::Object() : _M_package(nullptr), _M_references(0), _M_instance_index(Constants::index_none)
    {
        _M_owner                   = nullptr;
        ObjectArray& objects_array = get_instances_array();
        _M_instance_index          = objects_array.size();

        if (!get_free_indexes_array().empty())
        {
            _M_instance_index = get_free_indexes_array().back();
            get_free_indexes_array().pop_back();
            objects_array[_M_instance_index] = this;
        }
        else
        {
            objects_array.push_back(this);
        }

        _M_flags = 0;
        flag(Flag::IsSerializable, true);
        flag(Flag::IsAvailableForGC, _M_next_available_for_gc);
        _M_next_available_for_gc = false;
    }

    ENGINE_EXPORT HashIndex Object::hash_of_name(const String& name)
    {
        return memory_hash_fast(name.c_str(), name.length(), 0);
    }

    HashIndex Object::hash_index() const
    {
        return _M_name.hash();
    }

    ENGINE_EXPORT String Object::decode_name(const std::type_info& info)
    {
        return Engine::Demangle::decode_name(info);
    }

    ENGINE_EXPORT String Object::decode_name(const String& name)
    {
        return Engine::Demangle::decode_name(name);
    }

    ENGINE_EXPORT Package* Object::load_package(const String& name)
    {
        // Try to find package
        Package* package = Object::find_object_checked<Package>(name);

        if (package != nullptr)
            return package;

        package = find_package(name, true);

        if (package != nullptr && !package->load())
        {
            delete package;
            return nullptr;
        }

        return package;
    }

    ENGINE_EXPORT String Object::package_name_of(const String& name)
    {
        auto index = name.find_last_of(Constants::name_separator);
        if (index == String::npos)
            return "";

        index -= 1;
        return name.substr(0, index);
    }

    ENGINE_EXPORT String Object::object_name_of(const String& name)
    {
        auto pos = name.find_last_of(Constants::name_separator);
        if (pos == String::npos)
        {
            return name;
        }

        pos += Constants::name_separator.length() - 1;
        return name.substr(pos, name.length() - pos);
    }

    String Object::decode_name() const
    {
        return decode_name(typeid(*this));
    }

    const Object& Object::remove_from_instances_array() const
    {
        if (_M_instance_index < get_instances_array().size())
        {
            get_instances_array()[_M_instance_index] = nullptr;
            if (!engine_instance->is_shuting_down())
            {
                get_free_indexes_array().push_back(_M_instance_index);
            }
            _M_instance_index = Constants::max_size;
        }
        return *this;
    }


    ENGINE_EXPORT const ObjectArray& Object::all_objects()
    {
        return get_instances_array();
    }

    Object::~Object()
    {
        if (flag(Flag::IsDestructed) == false)
        {
            remove_from_instances_array();
            flag(Flag::IsDestructed, true);
        }
    }

    const String& Object::string_name() const
    {
        return _M_name.to_string();
    }


    ObjectRenameStatus Object::name(const char* new_name, size_t name_len, bool autorename)
    {
        if (std::strcmp(_M_name.to_string().c_str(), new_name) == 0)
            return ObjectRenameStatus::Skipped;

        Package* package_backup = _M_package;
        Name name_backup        = _M_name;

        auto restore_object_name = [&package_backup, &name_backup, this]() {
            _M_name = name_backup;
            if (package_backup)
            {
                package_backup->add_object(this);
            }
        };


        Package* package = _M_package;
        if (package)
        {
            package->remove_object(this);
        }

        // Find package
        const char* end_name         = new_name + name_len;
        const String& separator_text = Constants::name_separator;
        const size_t separator_len   = separator_text.length();

        const char* separator = Strings::strnstr(new_name, name_len, separator_text.c_str(), separator_len);

        if (separator)
        {
            package = root_package();
        }

        while (separator && package)
        {
            size_t next_name_size = static_cast<size_t>(separator - new_name);
            Package* next_package = package->find_object_checked<Package>(new_name, next_name_size, false);

            if (next_package == nullptr)
            {
                next_package = Object::new_instance<Package>();
                next_package->name(new_name, next_name_size);
                if (!package->add_object(next_package))
                {
                    restore_object_name();
                    return ObjectRenameStatus::Failed;
                }
            }

            package   = next_package;
            new_name  = separator + separator_len;
            separator = Strings::strnstr(new_name, end_name - new_name, separator_text.c_str(), separator_len);
        }

        // Apply new object name
        _M_name = Name(new_name, end_name - new_name);

        if (package)
        {
            if (!package->add_object(this))
            {
                restore_object_name();
                return ObjectRenameStatus::Failed;
            }
        }

        return ObjectRenameStatus::Success;
    }


    ObjectRenameStatus Object::name(const char* new_name, bool autorename)
    {
        return name(new_name, std::strlen(new_name), autorename);
    }

    ObjectRenameStatus Object::name(const String& new_name, bool autorename)
    {
        return name(new_name.c_str(), new_name.length(), autorename);
    }

    const Name& Object::name() const
    {
        return _M_name;
    }

    Object* Object::copy()
    {
        if (flag(Flag::IsDestructed))
            return nullptr;

        Object* object = nullptr;

        {
            object           = class_instance()->create_object();
            object->_M_flags = _M_flags;
        }

        return object;
    }

    bool Object::add_to_package(Package* package, bool autorename)
    {
        return package->add_object(this, autorename);
    }

    Object& Object::remove_from_package()
    {
        if (_M_package && _M_package != _M_root_package)
            _M_package->remove_object(this);
        return *this;
    }

    Object* Object::noname_object()
    {
        static thread_local byte data[sizeof(Object)];
        return reinterpret_cast<Object*>(data);
    }

    Package* Object::package() const
    {
        return _M_package;
    }

    String Object::full_name() const
    {
        String result =
                _M_name.is_valid() ? static_cast<String>(_M_name) : Strings::format("Noname object {}", _M_instance_index);
        Package* parent = this->_M_package;

        if (parent && parent != _M_root_package && !is_instance_of<Package>())
        {
            result = (parent->_M_name.to_string() + Constants::name_separator) + result;
            parent = parent->_M_package;
        }

        while (parent && parent != _M_root_package)
        {
            result = (parent->_M_name.to_string() + Constants::name_separator) + result;
            parent = parent->_M_package;
        }

        return result;
    }

    Counter Object::references() const
    {
        return _M_references;
    }

    void Object::add_reference()
    {
        ++_M_references;
    }

    void Object::remove_reference()
    {
        --_M_references;
    }

    const decltype(Object::_M_flags)& Object::flags() const
    {
        return _M_flags;
    }

    Object& Object::flag(Flag flag, bool status)
    {
        if (status)
        {
            _M_flags |= flag;
        }
        else
        {
            _M_flags &= ~flag;
        }
        return *this;
    }

    bool Object::flag(Flag flag) const
    {
        return (_M_flags & flag) == flag;
    }

    bool Object::has_all(Flags flags) const
    {
        return (_M_flags & flags) == flags;
    }

    bool Object::has_any(Flags flags) const
    {
        return (_M_flags & flags) != 0;
    }

    bool Object::is_noname() const
    {
        return !_M_name.is_valid();
    }


    ENGINE_EXPORT const Package* root_package()
    {
        return _M_root_package;
    }

    ENGINE_EXPORT Object* Object::find_object(const String& object_name)
    {
        return _M_root_package->find_object(object_name);
    }

    Object& Object::preload()
    {
        return *this;
    }

    Object& Object::postload()
    {
        return *this;
    }

    Object* Object::owner() const
    {
        return _M_owner;
    }

    Object& Object::owner(Object* new_owner)
    {
        _M_owner = new_owner;
        return *this;
    }

    Package* Object::find_package(const String& name, bool create)
    {
        return find_package(name.c_str(), name.length(), create);
    }

    Package* Object::find_package(const char* new_name, bool create)
    {
        return find_package(new_name, std::strlen(new_name), create);
    }


    static Package* find_next_package(Package* package, const char* name, size_t len, bool create)
    {
        Package* next_package = package->find_object_checked<Package>(name, len, false);

        if (next_package == nullptr && create)
        {
            next_package = Object::new_instance<Package>();
            next_package->name(name, len);
            package->add_object(next_package);
        }

        return next_package;
    }

    Package* Object::find_package(const char* new_name, size_t len, bool create)
    {
        Package* package = const_cast<Package*>(root_package());

        const char* end_name         = new_name + len;
        const String& separator_text = Constants::name_separator;
        const size_t separator_len   = separator_text.length();

        const char* separator = Strings::strnstr(new_name, len, separator_text.c_str(), separator_len);

        while (separator && package)
        {
            size_t next_name_size = static_cast<size_t>(separator - new_name);
            package               = find_next_package(package, new_name, next_name_size, create);
            new_name              = separator + separator_len;
            separator             = Strings::strnstr(new_name, end_name - new_name, separator_text.c_str(), separator_len);
        }

        return package ? find_next_package(package, new_name, end_name - new_name, create) : nullptr;
    }


    String Object::as_string() const
    {
        return Strings::format("{}: {}", class_instance()->name(), _M_name.to_string());
    }

    Index Object::instance_index() const
    {
        return _M_instance_index;
    }

    bool Object::archive_process(Archive* archive)
    {
        if (!SerializableObject::archive_process(archive))
        {
            return false;
        }

        return flag(Flag::IsSerializable);
    }

    bool Object::is_valid() const
    {
        if (has_any(IsUnreachable))
            return false;
        return true;
    }

    Path Object::filepath() const
    {
        const Package* pkg = instance_cast<Package>();
        if (!pkg)
        {
            pkg = _M_package;
        }

        if (!pkg || pkg == root_package())
            return {};

        Path path = pkg->string_name() + Constants::package_extention;
        pkg       = pkg->package();

        while (pkg && pkg != root_package())
        {
            path = Path(pkg->string_name()) / path;
            pkg  = pkg->package();
        }

        return path;
    }

    Package* Object::root_package()
    {
        Object::create_default_package();
        return _M_root_package;
    }
}// namespace Engine
