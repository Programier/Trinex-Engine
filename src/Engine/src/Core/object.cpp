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
#include <typeinfo>


namespace Engine
{

    static thread_local bool _M_next_available_for_gc = false;

    void Object::private_bind_class(class Class* c)
    {
        ScriptClassRegistrar registrar(c);

        String factory = Strings::format("{}@ f()", c->name());

        if (!c->has_all_flags(Class::IsSingletone))
        {
            registrar.behave(ScriptClassBehave::Factory, factory.c_str(), c->static_constructor());
        }

        registrar.require_type("Engine::Package")
                .behave(ScriptClassBehave::AddRef, "void f()", &Object::add_reference)
                .behave(ScriptClassBehave::Release, "void f()", &Object::remove_reference)
                .method("Package@ root_package()", &Object::root_package)
                .method("const string& string_name() const", func_of<const String&>(&Object::string_name))
                .method("ObjectRenameStatus name(string, bool) const", func_of<ObjectRenameStatus>(&Object::name))
                .method("string as_string() const", &Object::as_string)
                .method("bool add_to_package(Package@, bool)", &Object::add_to_package)
                .method("Package@ find_package(const string& in, bool)", &Object::find_package)
                .method("Object@ find_object(const string& in)", &Object::find_object)
                .method("Object& remove_from_package()", &Object::remove_from_package);
    }

    implement_class(Object, "Engine");

    implement_initialize_class(Object)
    {
        ScriptEnumRegistrar("Engine::ObjectRenameStatus")
                .set("Skipped", static_cast<int_t>(ObjectRenameStatus::Skipped))
                .set("Success", static_cast<int_t>(ObjectRenameStatus::Success))
                .set("Failed", static_cast<int_t>(ObjectRenameStatus::Failed));

        private_bind_class(This::static_class_instance());
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
            _M_root_package = new Package("Root Package");
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


    Object::Object()
        : _M_package(nullptr), _M_references(0), _M_index_in_package(Constants::index_none),
          _M_instance_index(Constants::index_none)
    {
        _M_owner = nullptr;
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

    ENGINE_EXPORT Object* Object::load_object(const String& name)
    {
        return Package::find_package(package_name_of(name), true)->load_object(object_name_of(name));
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

    ObjectRenameStatus Object::name(String new_name, bool autorename)
    {
        if (_M_name == new_name)
            return ObjectRenameStatus::Skipped;

        Package* package    = _M_package;
        String package_name = package_name_of(new_name);

        if (!package_name.empty())
        {
            package  = Package::find_package(package_name, true);
            new_name = Object::object_name_of(new_name);
        }

        if (!autorename && package && package->contains_object(new_name))
        {
            error_log("Object", "Failed to rename object. Object with name '%s' already exist in package '%s'",
                      new_name.c_str(), _M_package->string_name().c_str());
            return ObjectRenameStatus::Failed;
        }

        if (_M_package)
        {
            _M_package->remove_object(this);
        }

        _M_name = new_name;

        if (package)
            return Object::add_to_package(package, autorename) ? ObjectRenameStatus::Success
                                                               : ObjectRenameStatus::Failed;

        return ObjectRenameStatus::Success;
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
        String result   = _M_name.is_valid() ? static_cast<String>(_M_name)
                                             : Strings::format("Noname object {}", _M_instance_index);
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
        return _M_root_package->find_object(object_name, true);
    }

    bool Object::can_destroy(MessageList& messages)
    {
        if (_M_references > 0)
        {
            messages.push_back(Strings::format("Object {} is referenced", string_name()));
            return false;
        }

        if (this == _M_root_package)
        {
            messages.push_back("Cannot destroy root package!");
            return false;
        }

        return true;
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
        Package* package = const_cast<Package*>(root_package());

        Index prev_index = 0;
        Index index      = 0;

        while ((index = name.find_first_of(Constants::name_separator,
                                           prev_index + (prev_index ? Constants::name_separator.length() : 0))) !=
               String::npos)
        {
            if (index == String::npos)
            {
                break;
            }

            String package_name = name.substr(prev_index, index - prev_index);
            if (package_name.empty())
                return nullptr;
            prev_index = index;

            Object* object = package->find_object(package_name, false);
            if (object)
            {
                Package* new_package = object->instance_cast<Package>();
                if (new_package == nullptr)
                {
                    error_log("Object",
                              "Failed to create new package with name '%s'. Object already exist in "
                              "package '%s'!",
                              package_name.c_str(), package->full_name().c_str());
                    return nullptr;
                }
                package = new_package;
            }
            else
            {
                package = Object::new_instance_named<Package>(package_name, package);
            }
        }

        if (name[prev_index] == Constants::name_separator[0])
            prev_index += Constants::name_separator.length();
        String new_name        = name.substr(prev_index, name.length() - prev_index);
        Object* founded_object = package->find_object(new_name, false);
        if (founded_object == nullptr)
        {
            return create ? Object::new_instance_named<Package>(new_name, package) : nullptr;
        }

        Package* new_package =
                founded_object->flag(Flag::IsPackage) ? reinterpret_cast<Package*>(founded_object) : nullptr;
        if (new_package == nullptr)
        {
            error_log("Object",
                      "Failed to create new package with name '%s'. Object already exist in "
                      "package '%s'!",
                      new_name.c_str(), package->full_name().c_str());
        }

        return new_package;
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

    Package* Object::root_package()
    {
        Object::create_default_package();
        return _M_root_package;
    }
}// namespace Engine
