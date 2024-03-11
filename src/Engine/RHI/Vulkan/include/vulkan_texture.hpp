#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>


namespace Engine
{
    struct VulkanTexture : RHI_Texture {

        const Texture* m_engine_texture = nullptr;

        vk::Image m_image;
        vk::DeviceMemory m_image_memory;
        vk::ImageView m_image_view;
        vk::Format m_vulkan_format;
        vk::ComponentMapping m_swizzle;
        VkDescriptorSet m_imgui_descriptor_set = 0;

        VulkanTexture& create(const Texture* texture, const byte* data, size_t size);
        VulkanTexture& destroy();

        vk::ImageSubresourceRange subresource_range(MipMapLevel base);
        vk::ImageAspectFlags aspect() const;
        uint_t layer_count() const;
        vk::ImageViewType view_type() const;
        uint_t pixel_type_size() const;

        void generate_mipmap() override;
        void bind(BindLocation location) override;
        void update_texture(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, uint_t layer, const byte* data,
                            size_t data_size);
        void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data,
                               size_t data_size) override;
        bool can_use_color_as_color_attachment() const;
        bool is_depth_stencil_image() const;
        vk::Offset2D get_mip_size(MipMapLevel level) const;
        vk::ImageView get_image_view(const vk::ImageSubresourceRange& range);


        ~VulkanTexture();
    };
}// namespace Engine
