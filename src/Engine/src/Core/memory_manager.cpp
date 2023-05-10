#include <Core/class.hpp>
#include <Core/config.hpp>
#include <Core/logger.hpp>
#include <Core/memory_manager.hpp>
#include <Core/object.hpp>

namespace Engine
{

    MemoryManager::MemoryManager() = default;

    MemoryManager& MemoryManager::instance()
    {
        static MemoryManager collector;
        return collector;
    }

    void MemoryManager::free_object(Object* object)
    {
        if (!object->flag(ObjectFlags::OF_IsCollectedByGC) && object->flag(ObjectFlags::OF_NeedDelete))
        {
            object->internal_set_flag(ObjectFlags::OF_IsCollectedByGC, true);
            _M_objects[object->class_instance()->instance_size()].push_back(object);
            ++_M_count;

            // Force call destructor
            std::destroy_at(object);

            if (_M_count > static_cast<size_t>(engine_config.max_gc_collected_objects))
            {
                collect_garbage();
            }
        }
    }

    Object* MemoryManager::find_memory(size_t size)
    {

        // Try to find unused object
        auto it = _M_objects.lower_bound(size);

        if (it != _M_objects.end() && !it->second.empty())
        {
            Object* object = it->second.front();
            it->second.pop_front();

            force_call_destructor(object);
            return object;
        }


        byte* memory                         = allocator.allocate(size + sizeof(size_t));
        (*reinterpret_cast<size_t*>(memory)) = size;

        return reinterpret_cast<Object*>(memory + sizeof(size_t));
    }

    void MemoryManager::force_destroy_object(Object* object)
    {
        force_call_destructor(object);
        byte* memory = reinterpret_cast<byte*>(object) - sizeof(size_t);
        allocator.deallocate(memory, *reinterpret_cast<size_t*>(memory));
    }

    void MemoryManager::force_call_destructor(Object* object)
    {
        if (!object->flag(ObjectFlags::OF_Destructed))
        {
            object->~Object();
        }
    }

    void MemoryManager::collect_garbage()
    {
        for (auto& pair : _M_objects)
        {
            for (auto object : pair.second)
            {
                force_destroy_object(object);
            }
        }

        _M_count = 0;
        _M_objects.clear();
    }
}// namespace Engine
