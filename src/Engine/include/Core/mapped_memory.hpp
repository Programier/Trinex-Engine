#pragma once
#include <Core/engine_types.hpp>
#include <Core/exception.hpp>

namespace Engine
{
    template<typename ComponentType>
    class TypedMappedMemory
    {
    public:
        using value_type = ComponentType;
        ComponentType* _M_memory = nullptr;
        size_t _M_size           = 0;

    public:
        TypedMappedMemory(ComponentType* memory = nullptr, size_t size = 0) : _M_memory(memory), _M_size(size)
        {}

        template<typename OtherType>
        TypedMappedMemory(const TypedMappedMemory<OtherType>& memory)
            : TypedMappedMemory(reinterpret_cast<ComponentType*>(memory._M_memory),
                                (memory._M_size * sizeof(OtherType)) / sizeof(ComponentType))
        {}

        TypedMappedMemory(const TypedMappedMemory&) = default;
        TypedMappedMemory(TypedMappedMemory&&)      = default;

        TypedMappedMemory& operator=(const TypedMappedMemory&) = default;
        TypedMappedMemory& operator=(TypedMappedMemory&&)      = default;

        template<typename OtherType>
        TypedMappedMemory& operator=(const TypedMappedMemory<OtherType>& memory)
        {
            _M_memory = reinterpret_cast<ComponentType*>(memory._M_memory);
            _M_size = (memory._M_size * sizeof(OtherType)) / sizeof(ComponentType);
            return *this;
        }

        TypedMappedMemory& operator = (const std::initializer_list<ComponentType>& list)
        {
            size_t size = glm::min(_M_size, list.size());

            size_t index = 0;
            for(auto& ell : list)
            {
                if(index == size)
                    break;

                _M_memory[index++] = ell;
            }
            return *this;
        }

        inline ComponentType* data()
        {
            return _M_memory;
        }

        inline const ComponentType* data() const
        {
            return _M_memory;
        }

        inline size_t size() const
        {
            return _M_size;
        }

        inline ComponentType& operator[](Index index)
        {
            if (index >= _M_size)
                throw EngineException("MappedMemory: Index out of range!");
            return _M_memory[index];
        }

        const ComponentType& operator[](Index index) const
        {
            if (index >= _M_size)
                throw EngineException("MappedMemory: Index out of range!");
            return _M_memory[index];
        }

        ComponentType* begin()
        {
            return _M_memory;
        }

        ComponentType* end()
        {
            return _M_memory + _M_size;
        }

        const byte* begin() const
        {
            return _M_memory;
        }

        const byte* end() const
        {
            return _M_memory + _M_size;
        }

        const byte* cbegin() const
        {
            return _M_memory;
        }

        const byte* cend() const
        {
            return _M_memory + _M_size;
        }
    };

    using MappedMemory = TypedMappedMemory<byte>;

}// namespace Engine
