#include <vulkan_api.hpp>
#include <vulkan_types.hpp>


namespace Engine
{
    void VulkanAPI::line_width(float w)
    {
        current_command_buffer().setLineWidth(w);
    }
}// namespace Engine
