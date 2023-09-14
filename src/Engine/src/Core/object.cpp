#include <Core/api_object.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/demangle.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>
#include <typeinfo>


namespace Engine
{
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
                .method("const string& name() const", func_of<const String&>(&Object::name))
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

    Object& Object::mark_as_allocate_by_constroller()
    {
        trinex_flag(TrinexObjectFlags::IsOnHeap, true);
        trinex_flag(TrinexObjectFlags::IsAllocatedByController, true);
        return *this;
    }

    void Object::create_default_package()
    {
        if (_M_root_package == nullptr)
        {
            _M_root_package = new (MemoryManager::instance().find_memory<Package>()) Package("Root Package");
            _M_root_package->mark_as_allocate_by_constroller();
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

    Object& Object::update_hash()
    {
        _M_hash = Object::hash_of_name(_M_name);
        return *this;
    }

    Object& Object::insert_to_default_package()
    {
        create_default_package();
        _M_root_package->add_object(this, true);
        return *this;
    }


    bool Object::private_check_instance(const Class* const check_class) const
    {
        auto _class = class_instance();
        return _class != nullptr && _class->contains_class(check_class);
    }

    Object::Object()
        : _M_package(nullptr), _M_references(0), _M_hash(Constants::invalid_hash),
          _M_index_in_package(Constants::index_none), _M_instance_index(Constants::index_none)
    {
        ObjectArray& objects_array = get_instances_array();
        _M_instance_index          = objects_array.size();

        _M_name = Strings::format("Instance {}", _M_instance_index);
        update_hash();

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

        _M_trinex_flags.reset();
        trinex_flag(TrinexObjectFlags::IsSerializable, true);
        trinex_flag(TrinexObjectFlags::IsOnHeap, !EngineInstance::is_on_stack(this));
    }

    ENGINE_EXPORT HashIndex Object::hash_of_name(const String& name)
    {
        static Hash<String> hash_function;
        return hash_function(name);
    }

    HashIndex Object::hash_index() const
    {
        return _M_hash;
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
            package->mark_for_delete(true);
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

    void Object::delete_instance()
    {
        if (trinex_flag(TrinexObjectFlags::IsNeedDelete) && trinex_flag(TrinexObjectFlags::IsAllocatedByController))
        {
            remove_from_instances_array();
            debug_log("Garbage Collector", "Delete object instance '%s' with type '%s' [%p]\n", name().c_str(),
                      decode_name().c_str(), this);

            MemoryManager::instance().force_destroy_object(this);
        }
    }

    ENGINE_EXPORT const ObjectArray& Object::all_objects()
    {
        return get_instances_array();
    }

    Object::~Object()
    {
        remove_from_instances_array();
        if (_M_package)
        {
            _M_package->remove_object(this);
        }

        trinex_flag(TrinexObjectFlags::IsDestructed, true);
    }


    bool Object::mark_for_delete(bool skip_check)
    {
        static auto mark = [](Object* object) -> bool {
            if (!object->trinex_flag(TrinexObjectFlags::IsNeedDelete) &&
                object->trinex_flag(TrinexObjectFlags::IsAllocatedByController) && object->_M_references == 0)
            {
                object->remove_from_instances_array();

                if (object->_M_package)
                {
                    object->_M_package->remove_object(object);
                }

                object->trinex_flag(TrinexObjectFlags::IsNeedDelete, true);
                MemoryManager::instance().free_object(object);
                return true;
            }
            return false;
        };

        if (trinex_flag(TrinexObjectFlags::IsDestructed) || trinex_flag(TrinexObjectFlags::IsNeedDelete))
            return false;

        if (!skip_check)
        {
            MessageList errors;
            if (!can_destroy(errors))
            {
                error_log("Object", Strings::format("Cannot delete object '{}'", name()), errors);
                return false;
            }
        }

        Package* package = instance_cast<Package>();
        bool status      = true;

        if (package)
        {
            Vector<Object*>& objects = const_cast<Vector<Object*>&>(package->objects());
            while (!objects.empty())
            {
                Object* current_object = objects.front();
                if (!current_object->mark_for_delete(true))
                    status = false;
                else
                    package->remove_object(current_object);
            }
        }

        if (status)
        {
            status = mark(this);
        }

        return status;
    }

    bool Object::is_on_heap() const
    {
        return trinex_flag(TrinexObjectFlags::IsOnHeap);
    }

    const String& Object::name() const
    {
        return _M_name;
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
                      new_name.c_str(), _M_package->name().c_str());
            return ObjectRenameStatus::Failed;
        }

        if (_M_package)
        {
            _M_package->remove_object(this);
        }

        _M_name = new_name;
        update_hash();

        if (package)
            return Object::add_to_package(package, autorename) ? ObjectRenameStatus::Success
                                                               : ObjectRenameStatus::Failed;

        return ObjectRenameStatus::Success;
    }

    ENGINE_EXPORT void Object::collect_garbage()
    {
        MemoryManager::instance().collect_garbage();
    }

    Object* Object::copy()
    {
        if (trinex_flag(TrinexObjectFlags::IsNeedDelete) || trinex_flag(TrinexObjectFlags::IsDestructed))
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

    bool Object::trinex_flag(TrinexObjectFlags flag) const
    {
        return _M_trinex_flags[flag];
    }

    const Object& Object::trinex_flag(TrinexObjectFlags flag, bool status) const
    {
        _M_trinex_flags[static_cast<size_t>(flag)] = status;

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
        String result   = _M_name;
        Package* parent = this->_M_package;

        if (parent && parent != _M_root_package && !is_instance_of<Package>())
        {
            result = (parent->_M_name + Constants::name_separator) + result;
            parent = parent->_M_package;
        }

        while (parent && parent != _M_root_package)
        {
            result = (parent->_M_name + Constants::name_separator) + result;
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

    Object& Object::flag(ObjectFlags flag, bool status)
    {
        _M_flags[static_cast<size_t>(flag)] = status;
        return *this;
    }

    bool Object::flag(ObjectFlags flag) const
    {
        return _M_flags[static_cast<size_t>(flag)];
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
            messages.push_back(Strings::format("Object {} is referenced", name()));
            return false;
        }

        if (this == _M_root_package)
        {
            messages.push_back("Cannot destroy root package!");
            return false;
        }

        return true;
    }

    void Object::post_init_components()
    {}

    Package* Object::find_package(const String& name, bool create)
    {
        Package* package = const_cast<Package*>(root_package());

        Index prev_index = 0;
        Index index      = 0;

        while ((index = name.find_first_of(Constants::name_separator,
                                           prev_index + Constants::name_separator.length()) != String::npos))
        {
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

        String new_name        = name.substr(index, name.length() - index);
        Object* founded_object = package->find_object(new_name, false);
        if (founded_object == nullptr)
        {
            return create ? Object::new_instance_named<Package>(new_name, package) : nullptr;
        }

        Package* new_package = founded_object->trinex_flag(TrinexObjectFlags::IsPackage)
                                       ? reinterpret_cast<Package*>(founded_object)
                                       : nullptr;
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
        return Strings::format("{}: {}", class_instance()->name(), _M_name);
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

        return trinex_flag(TrinexObjectFlags::IsSerializable);
    }


    /////////////////////////////// GARBAGE CONTROLLER  ///////////////////////////////

    void Object::force_garbage_collection()
    {
        if (!engine_instance->is_shuting_down())
            return;

        info_log("Object", "Triggered garbage collector!\n");

        Object::collect_garbage();
        MemoryManager& manager             = MemoryManager::instance();
        manager._M_disable_collect_garbage = true;


        Set<Object*> maybe_not_deleted_objects;

        PriorityIndex current_priority = 0;
        PriorityIndex next_priority    = 0;


        ObjectArray& objects    = get_instances_array();
        size_t count            = objects.size();
        size_t next_start_index = 0;
        size_t next_end_index   = 0;

        while (count - next_start_index != 0)
        {
            current_priority = next_priority;
            next_priority    = Constants::max_size;

            for (size_t i = next_start_index; i < count; i++)
            {
                Object* object = objects[i];
                if (object == nullptr)
                {
                    if (next_start_index == i)
                    {
                        next_start_index = i + 1;
                        next_end_index   = i;
                    }
                    continue;
                }

                if (object->_M_force_destroy_priority == current_priority)
                {
                    if (object->trinex_flag(TrinexObjectFlags::IsAllocatedByController))
                    {
                        debug_log("Garbage Collector[FORCE]", "Deleting instance '%s' with type '%s' [%p]\n",
                                  object->name().c_str(), object->decode_name().c_str(), object);
                        object->trinex_flag(TrinexObjectFlags::IsNeedDelete, true);
                        manager.free_object(object);
                        maybe_not_deleted_objects.erase(object);
                    }
                    else if (object->is_on_heap())
                    {
                        maybe_not_deleted_objects.insert(object);
                    }

                    object->remove_from_instances_array();
                    if (next_start_index == i)
                    {
                        next_start_index = i + 1;
                        next_end_index   = i;
                    }
                }
                else
                {
                    if (object->_M_force_destroy_priority < next_priority)
                        next_priority = object->_M_force_destroy_priority;
                    next_end_index = i;
                }
            }

            count = next_end_index + 1;
        }

        for (auto& object : maybe_not_deleted_objects)
        {
            error_log("Garbage Collector: Object '%s' with type '%s' is dynamicly allocated and must be deleted!",
                      object->_M_name.c_str(), object->decode_name().c_str());
        }
    }

    Package* Object::root_package()
    {
        Object::create_default_package();
        return _M_root_package;
    }
}// namespace Engine
