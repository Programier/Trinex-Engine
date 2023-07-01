#pragma once
#include <Core/engine_types.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{

#define OBJECT_OF(ID) reinterpret_cast<VulkanObject*>(ID)
#define GET_TYPE(TYPE, ID) reinterpret_cast<TYPE*>(OBJECT_OF(ID))
#define DESTROY_CALL(func, instance)                                                                                   \
    {                                                                                                                  \
        if (instance)                                                                                                  \
            API->_M_device.func(instance);                                                                             \
        instance = nullptr;                                                                                            \
    }

    struct VulkanObject {
        void* _M_instance_address = nullptr;

        Identifier ID();

        template<typename Type>
        Type* get_instance()
        {
            return static_cast<Type*>(_M_instance_address);
        }

        virtual ~VulkanObject();
    };
}// namespace Engine
