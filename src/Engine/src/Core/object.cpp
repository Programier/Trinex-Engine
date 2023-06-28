#include "sol/overload.hpp"
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
    static ClassMetaData<void> void_instance = nullptr;

    static ClassMetaData<Object> object_instance =
            &Class::register_new_class<Engine::Object>("Engine::Object")
                     .set("root_package", &Object::root_package)
                     .set("class_instance", &Object::class_instance)
                     .set("find_package", &Object::find_package)
                     .set("find_object", &Object::find_object)
                     .set("flag", sol::overload(static_cast<bool (Object::*)(ObjectFlags) const>(&Object::flag),
                                                static_cast<Object& (Object::*) (ObjectFlags, bool)>(&Object::flag)))
                     .set("references", &Object::references)
                     .set("full_name", &Object::full_name)
                     .set("package", &Object::package)
                     .set("remove_from_package", &Object::remove_from_package)
                     .set("decode_name", static_cast<String (Object::*)() const>(&Object::decode_name))
                     .set("load_package", Object::load_package)
                     .set("mark_for_delete", &Object::mark_for_delete)
                     .set("is_on_heap", &Object::is_on_heap)
                     .set("collect_garbage", Object::collect_garbage)
                     .set("name", sol::overload(static_cast<const String& (Object::*) () const>(&Object::name),
                                                static_cast<ObjectRenameStatus (Object::*)(const String&, bool)>(
                                                        &Object::name)))
                     .set("add_to_package", &Object::add_to_package)
                     .set("as_string", &Object::as_string);


    static ObjectSet& get_instance_list()
    {
        static ObjectSet list;
        return list;
    }

    static Package* _M_root_package = nullptr;


    Object& Object::mark_as_allocate_by_constroller()
    {
        trinex_flag(TrinexObjectFlags::IsOnHeap, true);
        trinex_flag(TrinexObjectFlags::IsAllocatedByController, true);
        return *this;
    }

    Object& Object::create_default_package()
    {
        if (_M_root_package == nullptr)
        {
            _M_root_package = new (MemoryManager::instance().find_memory<Package>()) Package("Root Package");
            _M_root_package->mark_as_allocate_by_constroller();
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
        trinex_flag(TrinexObjectFlags::IsSerializable, true);
        trinex_flag(TrinexObjectFlags::IsOnHeap, !EngineInstance::is_on_stack(this));
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
        if (index != String::npos)
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

    void Object::delete_instance()
    {
        if ((!trinex_flag(TrinexObjectFlags::IsOnHeap)))
        {
            get_instance_list().erase(const_cast<Object*>(this));
        }

        if (trinex_flag(TrinexObjectFlags::IsNeedDelete) && trinex_flag(TrinexObjectFlags::IsAllocatedByController))
        {
            debug_log("Garbage Collector: Delete object instance '%s' with type '%s' [%p]\n", name().c_str(),
                      decode_name().c_str(), this);

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

        trinex_flag(TrinexObjectFlags::IsDestructed, true);
    }


    bool Object::mark_for_delete(bool skip_check)
    {
        static auto mark = [](Object* object) -> bool {
            if (!object->trinex_flag(TrinexObjectFlags::IsNeedDelete) &&
                object->trinex_flag(TrinexObjectFlags::IsAllocatedByController) && object->_M_references == 0)
            {
                get_instance_list().erase(object);

                if (object->_M_package)
                {
                    object->_M_package->_M_objects.erase(object->name());
                    object->_M_package = nullptr;
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
                logger->error(Strings::format("Cannot delete object '{}'", name()), errors);
                return false;
            }
        }

        Package* package = instance_cast<Package>();
        bool status      = true;

        if (package)
        {
            Package::ObjectMap& objects = const_cast<Package::ObjectMap&>(package->objects());
            while (!objects.empty())
            {
                const auto& ell = objects.begin();
                if (!ell->second->mark_for_delete(true))
                    status = false;

                objects.erase(ell->first);
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
        if (_M_class == nullptr || trinex_flag(TrinexObjectFlags::IsNeedDelete) ||
            trinex_flag(TrinexObjectFlags::IsDestructed))
            return nullptr;

        Object* object = nullptr;

        {
            object           = _M_class->create_without_package();
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
        if (_M_class == nullptr && !trinex_flag(TrinexObjectFlags::IsUnregistered))
        {
            _M_class = ClassMetaDataHelper::find_class(typeid(*this));
            if (_M_class == nullptr)
            {
                trinex_flag(TrinexObjectFlags::IsUnregistered, true);
            }
        }

        return _M_class;
    }

    String Object::as_string() const
    {
        return Strings::format("{}: {}", class_instance()->_M_name, _M_name);
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


        Set<Object*> maybe_not_deleted_objects;

        while (!get_instance_list().empty())
        {
            Object* object = (*get_instance_list().begin());
            if (object->trinex_flag(TrinexObjectFlags::IsAllocatedByController))
            {
                debug_log("Garbage Collector[FORCE]: Deleting instance '%s' with type '%s' [%p]\n",
                          object->name().c_str(), object->decode_name().c_str(), object);
                object->trinex_flag(TrinexObjectFlags::IsNeedDelete, true);
                manager.free_object(object);
                maybe_not_deleted_objects.erase(object);
            }
            else if (object->is_on_heap())
            {
                maybe_not_deleted_objects.insert(object);
            }
            get_instance_list().erase(object);
        }


        for (auto& object : maybe_not_deleted_objects)
        {
            error_log("Garbage Collector: Object '%s' with type '%s' is dynamicly allocated and must be deleted!",
                      object->_M_name.c_str(), object->decode_name().c_str());
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
