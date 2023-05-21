#include <Core/api_object.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/decode_typeid_name.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>

#include <typeinfo>


#define MAX_GARBAGE_COLLECTION_OBJECTS 20000


namespace luabridge
{
    template<>
    struct Stack<Engine::ObjectRenameStatus> : Engine::LuaInterpretter::EnumWrapper<Engine::ObjectRenameStatus> {
    };

    template<>
    struct Stack<Engine::ObjectFlags> : Engine::LuaInterpretter::EnumWrapper<Engine::ObjectFlags> {
    };
}// namespace luabridge


namespace Engine
{
    static ClassMetaData<void> void_instance = nullptr;

    static ClassMetaData<Object> object_instance =
            &Class::register_new_class<Engine::Object, void>("Engine::Object")
                     .register_method("root_package", Object::root_package)
                     .register_method("class_instance", &Object::class_instance)
                     .register_method("find_package", Object::find_package)
                     .register_method("find_object", Object::find_object)
                     //.register_method("set_flag", static_cast<Object& (Object::*) (ObjectFlags, bool)>(&Object::flag))
                     //.register_method("get_flag", static_cast<bool (Object::*)(ObjectFlags) const>(&Object::flag))
                     .register_method("references", &Object::references)
                     .register_method("full_name", &Object::full_name)
                     .register_method("package", &Object::package)
                     .register_method("remove_from_package", &Object::remove_from_package)
                     .register_method("class_name", &Object::class_name)
                     .register_method("load_package", Object::load_package)
                     .register_method("class_hash", &Object::class_hash)
                     .register_method("mark_for_delete", &Object::mark_for_delete)
                     .register_method("is_on_heap", &Object::is_on_heap)
                     .register_method("collect_garbage", Object::collect_garbage)
                     .register_method("get_name", static_cast<const String& (Object::*) () const>(&Object::name))
                     .register_method("set_name",
                                      static_cast<ObjectRenameStatus (Object::*)(const String&, bool)>(&Object::name))
                     .register_method("add_to_package", &Object::add_to_package);


    static ObjectSet& get_instance_list()
    {
        static ObjectSet list;
        return list;
    }

    static Package* _M_root_package = nullptr;


    Object& Object::mark_as_on_heap_instance()
    {
        trinex_flag(TrinexObjectFlags::OF_IsOnHeap, true);
        return *this;
    }

    Object& Object::create_default_package()
    {
        if (_M_root_package == nullptr)
        {
            _M_root_package = new (MemoryManager::instance().find_memory<Package>()) Package("Root Package");
            _M_root_package->mark_as_on_heap_instance();
        }
        return *this;
    }

    bool Object::object_is_exist(Package* package, const String& name)
    {
        if (package)
        {
            bool status = package->objects().contains(name);
            if (status)
            {
                logger->error("Object: Cannot create new object. Object '%s' is exist in package '%s'!", name.c_str(),
                              package->full_name().c_str());
            }
            return status;
        }

        return false;
    }

    Object& Object::insert_to_default_package()
    {
        create_default_package();

        auto& objects = _M_root_package->objects();

        _M_name = Strings::format("Instance {}", objects.size());
        while (objects.contains(_M_name)) _M_name += "_new";
        _M_root_package->add_object(this);

        return *this;
    }


    bool Object::private_check_instance(const Class* const check_class) const
    {
        auto _class = class_instance();
        return _class != nullptr && _class->contains_class(check_class);
    }

    Object::Object()
    {
        get_instance_list().insert(this);
        _M_trinex_flags.reset();
        trinex_flag(TrinexObjectFlags::OF_IsSerializable, true);
    }

    std::size_t Object::class_hash() const
    {
        return typeid(*this).hash_code();
    }

    ENGINE_EXPORT String Object::decode_name(const std::type_info& info)
    {
        return Engine::decode_name(info);
    }

    ENGINE_EXPORT String Object::decode_name(const String& name)
    {
        return Engine::decode_name(name);
    }

    ENGINE_EXPORT Package* Object::load_package(const String& name)
    {
        // Try to find package
        Package* package = Object::find_object_checked<Package>(name);

        if (package != nullptr)
            return package;

        package = find_package(name, true);

        if (package != nullptr)
        {
            package->load();
        }

        return package;
    }

    ENGINE_EXPORT Object* Object::load_object(const String& name)
    {
        return Package::find_package(package_name_of(name), true)->load_object(object_name_of(name));
    }

    ENGINE_EXPORT String Object::package_name_of(const String& name)
    {
        return name.substr(0, name.find("->"));
    }

    ENGINE_EXPORT String Object::object_name_of(const String& name)
    {
        auto pos = name.find("->");
        if (pos == String::npos)
        {
            return "";
        }

        pos += 2;
        return name.substr(pos, name.length() - pos);
    }

    String Object::class_name() const
    {
        return decode_name(typeid(*this));
    }

    void Object::delete_instance()
    {
        if ((!trinex_flag(TrinexObjectFlags::OF_IsOnHeap)))
        {
            get_instance_list().erase(const_cast<Object*>(this));
        }

        if (trinex_flag(TrinexObjectFlags::OF_NeedDelete) && trinex_flag(TrinexObjectFlags::OF_IsOnHeap))
        {
            debug_log("Garbage Collector: Delete object instance '%s' with type '%s' [%p]\n", name().c_str(),
                      class_name().c_str(), this);

            MemoryManager::instance().force_destroy_object(this);
        }
    }

    ENGINE_EXPORT const ObjectSet& Object::all_objects()
    {
        return get_instance_list();
    }

    Object::~Object()
    {
        get_instance_list().erase(this);
        if (_M_package)
        {
            _M_package->_M_objects.erase(name());
            _M_package = nullptr;
        }

        trinex_flag(TrinexObjectFlags::OF_Destructed, true);
    }


    bool Object::mark_for_delete(bool skip_check)
    {
        static auto mark = [](Object* object) -> bool {
            if (!object->trinex_flag(TrinexObjectFlags::OF_NeedDelete) &&
                object->trinex_flag(TrinexObjectFlags::OF_IsOnHeap) && object->_M_references == 0)
            {
                get_instance_list().erase(object);

                if (object->_M_package)
                {
                    object->_M_package->_M_objects.erase(object->name());
                    object->_M_package = nullptr;
                }

                object->trinex_flag(TrinexObjectFlags::OF_NeedDelete, true);
                MemoryManager::instance().free_object(object);
                return true;
            }
            return false;
        };

        if (trinex_flag(TrinexObjectFlags::OF_Destructed) || trinex_flag(TrinexObjectFlags::OF_NeedDelete))
            return false;

        if (!skip_check)
        {
            MessageList errors;
            if (!can_destroy(errors))
            {
                logger->error(Strings::format("Cannot delete object '{}'", name()), errors);
                return false;
            }
        }

        Package* package = instance_cast<Package>();
        bool status      = true;

        if (package)
        {
            for (auto& ell : package->objects())
            {
                if (!ell.second->mark_for_delete(true))
                    status = false;
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
        return trinex_flag(TrinexObjectFlags::OF_IsOnHeap);
    }

    const String& Object::name() const
    {
        return _M_name;
    }

    ObjectRenameStatus Object::name(const String& new_name, bool autorename)
    {
        if (_M_name == new_name)
            return ObjectRenameStatus::Skipped;

        Package* package = _M_package;

        if (!autorename && package && package->objects().contains(new_name))
        {
            logger->error("Object: Failed to rename object. Object with name '%s' already exist in package '%s'",
                          new_name.c_str(), _M_package->name().c_str());
            return ObjectRenameStatus::Failed;
        }

        if (package)
        {
            package->_M_objects.erase(_M_name);
            _M_package = nullptr;
        }

        _M_name = new_name;

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
        if (_M_class == nullptr || trinex_flag(TrinexObjectFlags::OF_NeedDelete) ||
            trinex_flag(TrinexObjectFlags::OF_Destructed))
            return nullptr;

        Object* object = nullptr;

        {
            bool flag = _M_class->_M_disable_pushing_to_default_package;
            const_cast<Class*>(_M_class)->_M_disable_pushing_to_default_package = true;

            object = _M_class->create();

            const_cast<Class*>(_M_class)->_M_disable_pushing_to_default_package = flag;
            object->_M_flags                                                    = _M_flags;
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

    Object& Object::trinex_flag(TrinexObjectFlags flag, bool status)
    {
        _M_trinex_flags[static_cast<size_t>(flag)] = status;

        return *this;
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
            result = (parent->_M_name + "->") + result;
            parent = parent->_M_package;
        }

        while (parent && parent != _M_root_package)
        {
            result = (parent->_M_name + "::") + result;
            parent = parent->_M_package;
        }

        return result;
    }

    Counter Object::references() const
    {
        return _M_references;
    }

    const decltype(Object::_M_flags)& Object::flags() const
    {
        return _M_flags;
    }

    const Object& Object::flag(ObjectFlags flag, bool status)
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

    Package* Object::find_package(const String& name, bool create)
    {
        Package* package = const_cast<Package*>(root_package());

        Index prev_index = 0;
        Index index      = 0;

        while ((index = name.find_first_of("::", prev_index + 2) != String::npos))
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
                    logger->error("Object: Failed to create new package with name '%s'. Object already exist in "
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

        auto& package_objects = package->objects();

        String new_name = name.substr(index, name.length() - index);
        auto it         = package_objects.find(new_name);
        if (it == package_objects.end())
        {
            return create ? Object::new_instance_named<Package>(new_name, package) : nullptr;
        }

        Package* new_package = it->second->instance_cast<Package>();
        if (new_package == nullptr)
        {
            logger->error("Object: Failed to create new package with name '%s'. Object already exist in "
                          "package '%s'!",
                          new_name.c_str(), package->full_name().c_str());
        }

        return new_package;
    }

    const class Class* Object::class_instance() const
    {
        return _M_class;
    }

    bool Object::archive_process(Archive* archive)
    {
        return SerializableObject::archive_process(archive);
    }

    void* Object::operator new(std::size_t size, void* data)
    {
        return data;
    }

    void Object::operator delete(void* data)
    {
        logger->error("Object: Don't use operator delete! Use object->mark_for_delete() instead!");
    }


    /////////////////////////////// GARBAGE CONTROLLER  ///////////////////////////////
    static bool shutdown = false;

    void Object::force_garbage_collection()
    {
        if (!shutdown)
            return;

        info_log("Engine: Triggered garbage collector!\n");

        Object::collect_garbage();
        MemoryManager& manager             = MemoryManager::instance();
        manager._M_disable_collect_garbage = true;

        while (!get_instance_list().empty())
        {
            Object* object = (*get_instance_list().begin());
            object->trinex_flag(TrinexObjectFlags::OF_NeedDelete, true);
            manager.free_object(object);
            get_instance_list().erase(object);
        }
    }

    void call_force_garbage_collection()
    {
        shutdown = true;
        Object::force_garbage_collection();
    }

    Package* Object::root_package()
    {
        return _M_root_package;
    }

    static DestroyController controller(call_force_garbage_collection);


}// namespace Engine
