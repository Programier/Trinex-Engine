#pragma once
#include <Core/rhi_initializers.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{

#define OBJECT_OF(ID) reinterpret_cast<VulkanObject*>(ID)
#define GET_TYPE(TYPE, ID) reinterpret_cast<TYPE*>(OBJECT_OF(ID))


    struct VulkanObject {
        Identifier ID();
        virtual ~VulkanObject();
    };
}// namespace Engine
