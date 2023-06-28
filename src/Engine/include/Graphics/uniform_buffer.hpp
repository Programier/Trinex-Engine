#pragma once

#include <Core/api_object.hpp>
#include <Core/constants.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT UniformBuffer : public ApiObject
    {
    public:
        DynamicStruct uniform_struct;

        UniformBuffer& create(const DynamicStructInstance* buffer = nullptr);
        UniformBuffer& update(const DynamicStructInstance* buffer, size_t offset = 0,
                                     size_t size = Constants::max_size);
        UniformBuffer& bind(BindingIndex index, size_t offset = 0, size_t size = Constants::max_size);
    };

}// namespace Engine
