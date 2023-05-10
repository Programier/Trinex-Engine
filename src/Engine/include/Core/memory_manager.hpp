#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/export.hpp>

namespace Engine
{
    ENGINE_EXPORT class MemoryManager
    {
    private:
        Allocator<byte> allocator;
        TreeMap<size_t, List<Object*>> _M_objects;
        size_t _M_count = 0;

        MemoryManager();
        void free_object(class Object* object);

        Object* find_memory(size_t size);
        void force_destroy_object(Object* object);
        void force_call_destructor(Object* object);

    public:
        ENGINE_EXPORT static MemoryManager& instance();
        void collect_garbage();

        template<typename Instance>
        typename std::enable_if<is_object_based<Instance>::value, Instance*>::type find_memory()
        {
            return reinterpret_cast<Instance*>(find_memory(sizeof(Instance)));
        }

        friend class Object;
    };
}// namespace Engine
