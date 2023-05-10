#include <vulkan_object.hpp>


namespace Engine
{
    Identifier VulkanObject::ID()
    {
        return reinterpret_cast<Identifier>(this);
    }
}
