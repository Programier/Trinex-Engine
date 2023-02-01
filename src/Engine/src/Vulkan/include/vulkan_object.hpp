#pragma once
#include <vulkan/vulkan.hpp>

namespace Engine
{
    struct VulkanObject {
        virtual void* get_instance_data() = 0;
        template<typename Type>

        Type* get_instance()
        {
            return static_cast<Type*>(get_instance_data());
        }

        virtual ~VulkanObject();
    };
}
