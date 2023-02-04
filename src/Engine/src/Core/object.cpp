#include <Core/api_object.hpp>
#include <Core/decode_typeid_name.hpp>
#include <Core/destroy_controller.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/object_instance_flags.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>
#include <list>
#include <typeinfo>


#define MAX_GARBAGE_COLLECTION_OBJECTS 20000

namespace Engine
{
    static ObjectSet& get_instance_list()
    {
        static ObjectSet list;
        return list;
    }

    static std::list<Object*>& get_instance_list_for_delete()
    {
        static std::list<Object*> list;
        return list;
    }

    static Package* _M_root_package = nullptr;


    Object& Object::mark_as_on_heap_instance()
    {
        _M_is_on_heap = true;
        return *this;
    }

    Object::Object()
    {
        _M_flags = OI_None;
        get_instance_list().insert(this);
        if (_M_root_package == nullptr)
        {
            // Creating Root Package
            // Dirty hack - Fix recursion
            _M_root_package = reinterpret_cast<Package*>(1024);
            _M_root_package = Object::new_instance<Package>("Root Package");
        }
        else if (_M_root_package != reinterpret_cast<Package*>(1024))
        {
            auto& objects = _M_root_package->objects();
            _M_name = Strings::format("Instance {}", objects.size());
            while (objects.contains(_M_name)) _M_name += "_new";
            _M_root_package->add_object(this);
        }
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

    String Object::class_name() const
    {
        return decode_name(typeid(*this));
    }

    void Object::delete_instance(bool force_delete)
    {
        bool skip = has_any_flags(OI_SkipGarbageCollection);
        if ((force_delete || _M_need_delete) && _M_is_on_heap && !skip)
        {
            logger->log("Garbage Collector: Delete object instance '%s'\n", class_name().c_str());
            delete this;
        }

        if (force_delete && (!_M_is_on_heap || skip))
        {
            auto obj = this->instance_cast<ApiObject>();
            if (obj)
            {
                obj->destroy();
            }
            get_instance_list().erase(const_cast<Object*>(this));
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
            _M_package->remove_object(this);
    }


    bool Object::mark_for_delete()
    {
        static auto mark = [](Object* object) -> bool {
            if (!object->_M_need_delete && object->_M_is_on_heap && object->_M_references == 0)
            {
                get_instance_list_for_delete().push_back(object);
                object->_M_need_delete = true;
                if (get_instance_list_for_delete().size() == MAX_GARBAGE_COLLECTION_OBJECTS)
                    Object::collect_garbage();
                return true;
            }
            return false;
        };

        Package* package = instance_cast<Package>();
        bool status = true;

        if (package)
        {
            for (auto& ell : package->objects())
            {
                if (!ell.second->mark_for_delete())
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
        return _M_is_on_heap;
    }

    BitMask Object::flags() const
    {
        return _M_flags;
    }

    Object& Object::add_flags(BitMask flags)
    {
        _M_flags |= flags;
        return *this;
    }

    Object& Object::remove_flags(BitMask flags)
    {
        _M_flags &= ~flags;
        return *this;
    }

    bool Object::has_any_flags(BitMask flags) const
    {
        return (_M_flags & flags) != 0;
    }

    bool Object::has_all_flags(BitMask flags) const
    {
        return (_M_flags & flags) == flags;
    }

    const String& Object::name() const
    {
        return _M_name;
    }

    Object& Object::name(const String& name)
    {
        Package* package = _M_package;

        if (package)
            package->remove_object(this);

        _M_name = name;
        if (package)
            package->add_object(this);
        return *this;
    }

    ENGINE_EXPORT void Object::collect_garbage()
    {
        for (auto instance : get_instance_list_for_delete()) instance->delete_instance();
        get_instance_list_for_delete().clear();
    }

    Object& Object::copy(const Object* copy_from)
    {
        return *this;
    }

    Object& Object::add_to_package(Package* package)
    {
        if (package)
            package->add_object(this);
        return *this;
    }

    Object& Object::remove_from_package()
    {
        if (_M_package)
            _M_package->remove_object(this);
        return *this;
    }

    Package* Object::package() const
    {
        return _M_package;
    }

    String Object::full_name() const
    {
        String result = _M_name;
        Object* object = this->_M_package;

        while (object && object != _M_root_package)
        {
            result = (object->_M_name + "::") + result;
            object = object->_M_package;
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


    ENGINE_EXPORT void* Object::operator new(std::size_t size)
    {
        throw std::runtime_error(
                "Cannot create object by new operator! Please, use Engine::Object::new_instance method!");
    }

    ENGINE_EXPORT void* Object::operator new[](std::size_t size)
    {
        throw std::runtime_error(
                "Cannot create object by new operator! Please, use Engine::Object::new_instance method!");
    }


    ENGINE_EXPORT void* Object::operator new(std::size_t size, void* data)
    {
        return ::operator new(size, data);
    }

    ENGINE_EXPORT void* Object::operator new[](std::size_t size, void* data)
    {
        return ::operator new[](size, data);
    }


    /////////////////////////////// GARBAGE CONTROLLER  ///////////////////////////////
    static bool shutdown = false;
    void force_garbage_collection()
    {
        if (!shutdown)
            return;

        logger->log("Engine: Triggered garbage collector!\n");
        while (!get_instance_list().empty()) (*get_instance_list().begin())->delete_instance(true);
    }

    static void call_force_garbage_collection()
    {
        shutdown = true;
        force_garbage_collection();
    }

    static DestroyController controller(call_force_garbage_collection);

}// namespace Engine
