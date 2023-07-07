#include <Core/class.hpp>
#include <Core/engine_config.hpp>
#include <Core/logger.hpp>
#include <Core/memory_manager.hpp>
#include <Core/object.hpp>
#include <malloc.h>
#include <mutex>

static std::mutex mutex;

namespace Engine
{
    FORCE_INLINE size_t private_allocated_size()
    {
        return mallinfo2().uordblks;
    }

    MemoryManager::MemoryManager() = default;

    MemoryManager& MemoryManager::instance()
    {
        static MemoryManager collector;
        return collector;
    }

    void MemoryManager::free_object(Object* object)
    {
        if (!object->trinex_flag(TrinexObjectFlags::IsCollectedByGC) &&
            object->trinex_flag(TrinexObjectFlags::IsNeedDelete) &&
            object->trinex_flag(TrinexObjectFlags::IsAllocatedByController))
        {
            object->trinex_flag(TrinexObjectFlags::IsCollectedByGC, true);
            if (_M_disable_collect_garbage)
                _M_objects[0].push_back(object);
            else
                _M_objects[object->class_instance()->instance_size()].push_back(object);
            ++_M_count;

            force_call_destructor(object);

            if (!_M_disable_collect_garbage && _M_count > static_cast<size_t>(engine_config.max_gc_collected_objects))
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
        if (!object->trinex_flag(TrinexObjectFlags::IsDestructed))
        {
            std::destroy_at(object);
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

    MemoryManager::~MemoryManager()
    {
        collect_garbage();
    }

    size_t MemoryManager::allocated_size()
    {
        std::unique_lock lock(mutex);
        return private_allocated_size();
    }
}// namespace Engine
