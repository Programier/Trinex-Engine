#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>


namespace Engine
{
    static FORCE_INLINE Allocator<byte>& allocator()
    {
        static Allocator<byte> object_allocator;
        return object_allocator;
    }

    static FORCE_INLINE Set<Object*>& deferred_destroy_list()
    {
        static Set<Object*> deferred_destroy_list;
        return deferred_destroy_list;
    }

    void* Object::operator new(size_t size) noexcept
    {
        byte* memory = allocator().allocate(size);
        prepare_next_object_for_gc();
        return memory;
    }

    void Object::operator delete(void* _memory, size_t size) noexcept
    {
        byte* memory = reinterpret_cast<byte*>(_memory);
        allocator().deallocate(memory, size);
    }

    void Object::delete_object(Object* object)
    {
        if (!object->is_noname())
        {
            debug_log("Garbage Collector", "Destroy object '%s'", object->string_name().c_str());
        }
        else
        {
            debug_log("Garbage Collector", "Destroy noname object with type '%s'",
                      object->class_instance()->name().c_str());
        }

        delete object;
    }

    Object& Object::deferred_destroy()
    {
        deferred_destroy_list().insert(this);
        return *this;
    }


    static void force_destroy_all()
    {
        auto& objects = Object::all_objects();
        for (Object* object : objects)
        {
            if (object && object->has_any(Object::IsAvailableForGC))
                Object::delete_object(object);
        }
    }

    void Object::collect_garbage(GCFlag flag)
    {
        while (!deferred_destroy_list().empty())
        {
            Object* object = *deferred_destroy_list().begin();
            if (object && object->has_all(Object::IsAvailableForGC))
            {
                delete_object(object);
            }
            deferred_destroy_list().erase(object);
        }

        if (flag == GCFlag::DestroyAll)
        {
            force_destroy_all();
        }
    }

}// namespace Engine
