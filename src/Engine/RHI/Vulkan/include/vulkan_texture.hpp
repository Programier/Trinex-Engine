#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>


namespace Engine
{
    struct VulkanTexture : RHI_Texture {

        const Texture* _M_engine_texture = nullptr;

        vk::Image _M_image;
        vk::DeviceMemory _M_image_memory;
        vk::ImageView _M_image_view;
        vk::Format _M_vulkan_format;
        vk::ComponentMapping _M_swizzle;
        VkDescriptorSet _M_imgui_descriptor_set = 0;

        VulkanTexture& create(const Texture* texture, const byte* data);
        VulkanTexture& destroy();

        vk::ImageSubresourceRange subresource_range(MipMapLevel base);
        vk::ImageAspectFlags aspect() const;
        uint_t layer_count() const;
        vk::ImageViewType view_type() const;
        uint_t pixel_type_size() const;

        void generate_mipmap() override;
        void bind(BindLocation location) override;
        void bind_combined(RHI_Sampler* sampler, BindLocation location) override;
        void update_texture(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, uint_t layer, const byte* data);
        void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data) override;
        bool can_use_color_as_color_attachment() const;
        bool is_depth_stencil_image() const;
        vk::Offset2D get_mip_size(MipMapLevel level) const;
        vk::ImageView get_image_view(const vk::ImageSubresourceRange& range);


        ~VulkanTexture();
    };
}// namespace Engine
