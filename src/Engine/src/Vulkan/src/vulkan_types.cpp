#include <vulkan_types.hpp>


namespace Engine
{
    const std::unordered_map<IndexBufferComponent, vk::IndexType> _M_index_types{
            {IndexBufferComponent::UnsignedInt, vk::IndexType::eUint32},
            {IndexBufferComponent::UnsignedShort, vk::IndexType::eUint16},
            {IndexBufferComponent::UnsignedByte, vk::IndexType::eUint8EXT}};

    const std::unordered_map<Primitive, vk::PrimitiveTopology> _M_primitive_topologies{
            {Primitive::Point, vk::PrimitiveTopology::ePointList},
            {Primitive::Line, vk::PrimitiveTopology::eLineList},
            {Primitive::Triangle, vk::PrimitiveTopology::eTriangleList},
    };

    const std::unordered_map<BufferValueType, std::vector<vk::Format>> _M_formats{
            {BufferValueType::Float, std::vector{vk::Format::eR32Sfloat, vk::Format::eR32G32Sfloat,
                                                 vk::Format::eR32G32B32Sfloat, vk::Format::eR32G32B32A32Sfloat}},

            //{BufferValueType::UNSIGNED_BYTE, vk::Format:}
            //{BufferValueType::UNSIGNED_SHORT, vk::Format:}
            //{BufferValueType::UNSIGNED_INT, vk::Format:}
            //{BufferValueType::SHORT, vk::Format:}
            //{BufferValueType::INT, vk::Format:}
            //{BufferValueType::BYTE, vk::Format:}
            //{BufferValueType::HALF_FLOAT, vk::Format:}
            //{BufferValueType::UNSIGNED_SHORT_5_6_5, vk::Format:}
            //{BufferValueType::UNSIGNED_SHORT_4_4_4_4, vk::Format:}
            //{BufferValueType::UNSIGNED_SHORT_5_5_5_1, vk::Format:}
            //{BufferValueType::UNSIGNED_INT_2_10_10_10_REV, vk::Format:}
            //{BufferValueType::UNSIGNED_INT_24_8, vk::Format:}
            //{BufferValueType::UNSIGNED_INT_10F_11F_11F_REV, vk::Format:}
            //{BufferValueType::UNSIGNED_INT_5_9_9_9_REV, vk::Format:}
            //{BufferValueType::FLOAT_32_UNSIGNED_INT_24_8_REV, vk::Format:}
    };

    const vk::Format _M_shader_data_types[19] = {
            vk::Format::eUndefined,         //  BOOL
            vk::Format::eR32Sint,           //  INT
            vk::Format::eR32Uint,           //  UINT
            vk::Format::eR32Sfloat,         //  FLOAT
            vk::Format::eR32G32Sfloat,      //  VEC2
            vk::Format::eR32G32B32Sfloat,   //  VEC3
            vk::Format::eR32G32B32A32Sfloat,//  VEC4
            vk::Format::eR32G32Sint,        //  IVEC2
            vk::Format::eR32G32B32Sint,     //  IVEC3
            vk::Format::eR32G32B32A32Sint,  //  IVEC4
            vk::Format::eR32G32Uint,        //  UVEC2
            vk::Format::eR32G32B32Uint,     //  UVEC3
            vk::Format::eR32G32B32A32Uint,  //  UVEC4
            vk::Format::eUndefined,         //  BVEC2
            vk::Format::eUndefined,         //  BVEC3
            vk::Format::eUndefined,         //  BVEC4
            vk::Format::eUndefined,         //  MAT2
            vk::Format::eUndefined,         //  MAT3
            vk::Format::eUndefined,         //  MAT4
    };

}// namespace Engine
