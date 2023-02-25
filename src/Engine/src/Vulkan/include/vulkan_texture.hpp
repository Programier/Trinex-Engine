#pragma once
#include <vulkan_object.hpp>


namespace Engine
{
    struct VulkanTexture : VulkanObject
    {
        vk::Image _M_image;
        vk::DeviceMemory _M_image_memory;
        vk::ImageView _M_image_view;
        vk::Sampler _M_texture_sampler;
        vk::ImageType _M_type;
        TextureParams _M_params;
        Size3D _M_size;
        size_t _M_mip_levels = 1;

        void * get_instance_data() override;

        VulkanTexture();
        VulkanTexture& init(const TextureParams& params);
        size_t pixel_component_count();
        vk::Format format();
        VulkanTexture& gen_texture_2D(const Size2D& size, int_t mipmap, void* data);
        ~VulkanTexture();

    private:
        VulkanTexture& create_image();
        VulkanTexture& create_image_view();
        VulkanTexture& create_texture_sampler();
        VulkanTexture& transition_image_layout(vk::ImageLayout old_layout, vk::ImageLayout new_layout);
        VulkanTexture& copy_buffer_to_image(vk::Buffer buffer);
        vk::ImageViewType image_view_type();
        vk::ImageSubresourceRange subresource_range();
    };
}
