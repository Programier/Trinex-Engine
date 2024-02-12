#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/property.hpp>

#define DISABLE_GC 1

namespace Engine
{
    enum class GCCollectStage
    {
        SetupUnreachable,
        FindUnreachable,
        CollectUnreachable,
    };

    static struct GCState {
        Index processed_object_index  = 0;
        size_t processed_object_count = 0;
        float last_destory_time       = 0.f;

        GCCollectStage collect_stage = GCCollectStage::SetupUnreachable;
    } gc_state;

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
            debug_log("Garbage Collector", "Destroy noname object with type '%s'", object->class_instance()->name().c_str());
        }

        delete object;
    }

    Object& Object::deferred_destroy()
    {
        deferred_destroy_list().insert(this);
        flags(IsWaitDestroy, true);
        return *this;
    }


    static void force_destroy_all()
    {
        auto& objects = Object::all_objects();
        for (Object* object : objects)
        {
            if (object && object->flags.has_any(Object::IsAvailableForGC))
                Object::delete_object(object);
        }
    }

    static size_t find_unreachable(Object* object)
    {
        size_t count = 1;

        if (object->references() > 0 || object->is_engine_resource())
        {
            object->flags(Object::IsUnreachable, false);
        }

        Object* owner = object->owner();

        if (owner)
        {
            if (owner->flags(Object::IsUnreachable) && !object->flags(Object::IsDefinetlyUnreachable))
            {
                count += find_unreachable(owner);
            }

            if (!owner->flags(Object::IsUnreachable))
            {
                object->flags(Object::IsUnreachable, false);
            }
        }

        if (object->flags(Object::IsUnreachable))
        {
            object->flags(Object::IsDefinetlyUnreachable, true);
        }
        else
        {
            Class* class_instance = object->class_instance();

            if (class_instance)
            {
                for (auto& prop : class_instance->properties())
                {
                    if (!prop)
                    {
                        continue;
                    }

                    auto type = prop->type();
                    if (type == Property::Type::Object || type == Property::Type::ObjectReference)
                    {
                        Object* object = prop->property_value(object).cast<Object*>();
                        if (object)
                        {
                            object->flags(Object::IsUnreachable, false);
                        }
                    }
                }
            }
        }

        return count;
    }

    static GCFlag collect_unreachable_objects()
    {
        const auto& objects = Object::all_objects();

        if (gc_state.processed_object_index >= objects.size())
        {
            gc_state.processed_object_index = 0;
            return GCFlag::DestroyGargabe;
        }

        gc_state.processed_object_count = 0;

        if (gc_state.collect_stage == GCCollectStage::SetupUnreachable)
        {
            while (gc_state.processed_object_count < engine_config.gc_max_object_per_tick &&
                   gc_state.processed_object_index < objects.size())
            {
                Object* object = objects[gc_state.processed_object_index];

                if (object && object->flags(Object::IsAvailableForGC))
                {
                    object->flags(Object::IsUnreachable, true);
                    ++gc_state.processed_object_count;
                }

                ++gc_state.processed_object_index;
            }

            if (gc_state.processed_object_index >= objects.size())
            {
                gc_state.processed_object_index = 0;
                gc_state.collect_stage          = GCCollectStage::FindUnreachable;
            }

            return GCFlag::DetectGarbage;
        }

        if (gc_state.collect_stage == GCCollectStage::FindUnreachable)
        {
            while (gc_state.processed_object_count < engine_config.gc_max_object_per_tick &&
                   gc_state.processed_object_index < objects.size())
            {
                Object* object = objects[gc_state.processed_object_index];

                if (object)
                {
                    gc_state.processed_object_count += find_unreachable(object);
                }

                ++gc_state.processed_object_index;
            }

            if (gc_state.processed_object_index >= objects.size())
            {
                gc_state.processed_object_index = 0;
                gc_state.collect_stage          = GCCollectStage::CollectUnreachable;
            }
            return GCFlag::DetectGarbage;
        }

        if (gc_state.collect_stage == GCCollectStage::CollectUnreachable)
        {
            return GCFlag::DetectGarbage;
        }

        return GCFlag::DestroyGargabe;
    }

    GCFlag Object::collect_garbage(GCFlag flag)
    {
        if (flag == GCFlag::None)
        {
            if (engine_instance->time_seconds() - gc_state.last_destory_time > engine_config.gc_wait_time)
            {
                return GCFlag::DetectGarbage;
            }

            return GCFlag::None;
        }

#if !DISABLE_GC
        if (flag == GCFlag::DetectGarbage)
        {
            return collect_unreachable_objects();
        }

        if (flag == GCFlag::DestroyGargabe)
        {
            size_t count = 0;

            while (!deferred_destroy_list().empty() && count < engine_config.gc_max_object_per_tick)
            {
                Object* object = *deferred_destroy_list().begin();
                if (object && object->flags(Object::IsAvailableForGC))
                {
                    delete_object(object);
                }
                deferred_destroy_list().erase(object);
                ++count;
            }

            if (deferred_destroy_list().empty())
            {
                gc_state.last_destory_time = engine_instance->time_seconds();
                return GCFlag::None;
            }
            else
            {
                return GCFlag::DestroyGargabe;
            }
        }
#endif

        if (flag == GCFlag::DestroyAll)
        {
            force_destroy_all();
        }

        return GCFlag::None;
    }

}// namespace Engine
