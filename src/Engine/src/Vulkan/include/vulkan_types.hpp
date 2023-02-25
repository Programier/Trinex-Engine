#pragma once
#include <unordered_map>
#include <Core/engine_types.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{
    extern const std::unordered_map<IndexBufferComponent, vk::IndexType> _M_index_types;
    extern const std::unordered_map<Primitive, vk::PrimitiveTopology> _M_primitive_topologies;
    extern const std::unordered_map<BufferValueType, std::vector<vk::Format>> _M_formats;
    extern const vk::Format _M_shader_data_types[19];
}
