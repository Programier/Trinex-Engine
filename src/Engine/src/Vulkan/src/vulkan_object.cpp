#include <vulkan_object.hpp>


namespace Engine
{
    ObjID VulkanObject::ID()
    {
        return reinterpret_cast<ObjID>(this);
    }
}
