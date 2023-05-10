#include <Core/buffer_types.hpp>
#include <stdexcept>


namespace Engine
{
#define id(type) typeid(type).hash_code()

    IndexBufferComponent index_buffer_component_of(const std::type_info& info)
    {
        static Map<std::size_t, IndexBufferComponent> _M_buffer_value_types = {
                {id(byte), IndexBufferComponent::UnsignedByte},
                {id(unsigned short), IndexBufferComponent::UnsignedShort},
                {id(unsigned int), IndexBufferComponent::UnsignedInt},
        };

        if (_M_buffer_value_types.contains(info.hash_code()))
            return _M_buffer_value_types.at(info.hash_code());
        throw std::runtime_error("Type not support!");
    }
}// namespace Engine
