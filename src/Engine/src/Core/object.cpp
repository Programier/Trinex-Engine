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
    template<>
    const Class* const ClassMetaData<void>::class_instance = nullptr;


    template<>
    FORCE_EXPORT const Class* const ClassMetaData<Engine::Object>::class_instance =
            &Class::register_new_class<Engine::Object, void>("Engine::Object")
                     .register_method("root_package", Object::root_package)
                     .register_method("class_instance", &Object::class_instance)
                     .register_method("find_package", Object::find_package)
                     .register_method("find_object", Object::find_object)
                     .register_method("set_flag", static_cast<Object& (Object::*) (ObjectFlags, bool)>(&Object::flag))
                     .register_method("get_flag", static_cast<bool (Object::*)(ObjectFlags) const>(&Object::flag))
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
                                      static_cast<ObjectRenameStatus (Object::*)(const String& name)>(&Object::name))
                     .register_method("add_to_package", &Object::add_to_package);


    static ObjectSet& get_instance_list()
    {
        static ObjectSet list;
        return list;
    }

    static Package* _M_root_package = nullptr;


    Object& Object::mark_as_on_heap_instance()
    {
        internal_set_flag(ObjectFlags::OF_IsOnHeap, true);
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
        return class_instance()->contains_class(check_class);
    }

    void Object::internal_set_flag(ObjectFlags flag, bool flag_value)
    {
        _M_flags[static_cast<size_t>(flag)] = flag_value;
    }

    Object::Object()
    {
        get_instance_list().insert(this);
        _M_flags.reset();
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

    String Object::class_name() const
    {
        return decode_name(typeid(*this));
    }

    void Object::delete_instance()
    {
        if ((!flag(ObjectFlags::OF_IsOnHeap)))
        {
            get_instance_list().erase(const_cast<Object*>(this));
        }

        if (flag(ObjectFlags::OF_NeedDelete) && flag(ObjectFlags::OF_IsOnHeap))
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

        internal_set_flag(ObjectFlags::OF_Destructed, true);
    }


    bool Object::mark_for_delete(bool skip_check)
    {
        static auto mark = [](Object* object) -> bool {
            if (!object->flag(ObjectFlags::OF_NeedDelete) && object->flag(ObjectFlags::OF_IsOnHeap) &&
                object->_M_references == 0)
            {
                get_instance_list().erase(object);

                if (object->_M_package)
                {
                    object->_M_package->_M_objects.erase(object->name());
                    object->_M_package = nullptr;
                }

                object->internal_set_flag(ObjectFlags::OF_NeedDelete, true);
                MemoryManager::instance().free_object(object);
                return true;
            }
            return false;
        };

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
        return flag(ObjectFlags::OF_IsOnHeap);
    }

    const String& Object::name() const
    {
        return _M_name;
    }

    ObjectRenameStatus Object::name(const String& new_name)
    {
        if (_M_name == new_name)
            return ObjectRenameStatus::Skipped;

        Package* package = _M_package;

        if (package && package->objects().contains(new_name))
        {
            logger->error("Object: Failed to rename object. Object with name '%s' already exist in package '%s'",
                          new_name.c_str(), _M_package->name().c_str());
            return ObjectRenameStatus::Failed;
        }

        if (package)
            package->remove_object(this);

        _M_name = new_name;
        if (package)
            package->add_object(this);
        return ObjectRenameStatus::Success;
    }

    ENGINE_EXPORT void Object::collect_garbage()
    {
        MemoryManager::instance().collect_garbage();
    }

    Object& Object::copy(const Object* copy_from)
    {
        return *this;
    }

    bool Object::add_to_package(Package* package, bool autorename)
    {
        if (package)
        {
            auto& objects = package->objects();

            if (autorename)
                while (objects.contains(_M_name)) _M_name += "_new";

            return package->add_object(this);
        }
        return false;
    }

    Object& Object::remove_from_package()
    {
        if (_M_package && _M_package != _M_root_package)
            _M_package->remove_object(this);
        return *this;
    }

    bool Object::flag(ObjectFlags flag) const
    {
        return _M_flags[flag];
    }

    Object& Object::flag(ObjectFlags flag, bool status)
    {
        if (static_cast<size_t>(flag) < static_cast<size_t>(ObjectFlags::__OF_PRIVATE_SECTION__))
        {
            logger->warning("Object: Cannot set private flag!");
        }
        else
        {
            _M_flags[static_cast<size_t>(flag)] = status;
        }

        return *this;
    }

    const decltype(Object::_M_flags)& Object::flags() const
    {
        return _M_flags;
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

    ENGINE_EXPORT const Package* root_package()
    {
        return _M_root_package;
    }

    ENGINE_EXPORT Object* Object::find_object(const String& object_name)
    {
        return _M_root_package->find_object_in_package(object_name, true);
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

            Object* object = package->find_object_in_package(package_name, false);
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

    bool Object::serialize(BufferWriter* writer)
    {
        if (!SerializableObject::serialize(writer))
            return false;

        bool status = writer->write(_M_name);
        if (!status)
        {
            logger->error("Object: Failed to serialize object '%s'", full_name().c_str());
        }

        return status;
    }

    ENGINE_EXPORT String Object::read_object_name(BufferReader* reader)
    {
        if (reader)
        {
            return reader->read_value<String>();
        }
        return "";
    }

    bool Object::deserialize(BufferReader* reader)
    {
        if (!SerializableObject::deserialize(reader))
            return false;

        bool status = reader->read(_M_name);
        if (!status)
        {
            logger->error("Object: Failed to deserialize object!");
        }
        return true;
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
    void force_garbage_collection()
    {
        if (!shutdown)
            return;

        info_log("Engine: Triggered garbage collector!\n");

        Object::collect_garbage();
        while (!get_instance_list().empty())
        {
            Object* object = (*get_instance_list().begin());
            object->internal_set_flag(ObjectFlags::OF_NeedDelete, true);
            object->delete_instance();
        }
    }

    static void call_force_garbage_collection()
    {
        shutdown = true;
        force_garbage_collection();
    }

    Package* Object::root_package()
    {
        return _M_root_package;
    }

    static DestroyController controller(call_force_garbage_collection);

}// namespace Engine
