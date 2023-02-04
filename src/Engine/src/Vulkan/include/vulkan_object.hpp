#pragma once
#include <vulkan/vulkan.hpp>
#include <Core/object_types.hpp>

namespace Engine
{

#define OBJECT_OF(ID) reinterpret_cast<VulkanObject*>(ID)
#define GET_TYPE(TYPE, ID) reinterpret_cast<TYPE*>(OBJECT_OF(ID))

    struct VulkanObject {
        virtual void* get_instance_data() = 0;

        ObjID ID();

        template<typename Type>
        Type* get_instance()
        {
            return static_cast<Type*>(get_instance_data());
        }

        virtual ~VulkanObject();
    };
}
