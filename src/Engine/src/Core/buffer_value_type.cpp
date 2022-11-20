#include <Core/buffer_value_type.hpp>
#include <stdexcept>
#include <unordered_map>

namespace Engine
{
#define id(type) typeid(type).hash_code()

    BufferValueType get_type_by_typeid(const std::type_info& info)
    {
        static std::unordered_map<std::size_t, BufferValueType> _M_buffer_value_types = {
                {id(float), BufferValueType::FLOAT},
                {id(byte), BufferValueType::UNSIGNED_BYTE},
                {id(unsigned short), BufferValueType::UNSIGNED_SHORT},
                {id(unsigned int), BufferValueType::UNSIGNED_INT},
                {id(short), BufferValueType::SHORT},
                {id(int), BufferValueType::INT},
        };

        if (_M_buffer_value_types.contains(info.hash_code()))
            return _M_buffer_value_types.at(info.hash_code());
        throw std::runtime_error("Type not support!");
    }
}// namespace Engine
