#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan/vulkan.hpp>


namespace Engine
{
    vk::Format parse_engine_format(ColorFormat format);

    struct VulkanTexture : RHI_Texture {
        vk::Extent2D size;

        vk::Image _M_image;
        vk::DeviceMemory _M_image_memory;
        vk::ImageView _M_image_view;
        vk::Format _M_vulkan_format;
        vk::ComponentMapping _M_swizzle;

        TextureType _M_type;
        ColorFormat _M_engine_format;
        uint_t _M_mipmap_count;
        uint_t _M_base_mip_level;

        VulkanTexture& create(const TextureCreateInfo& info, TextureType type, const byte* data);
        VulkanTexture& destroy();

        vk::ImageSubresourceRange subresource_range(MipMapLevel base);
        vk::ImageAspectFlags aspect() const;
        uint_t layer_count() const;
        vk::ImageViewType view_type() const;
        uint_t pixel_type_size() const;

        void generate_mipmap() override;
        void bind(BindingIndex binding, BindingIndex set) override;
        void bind_combined(RHI_Sampler* sampler, BindingIndex binding, BindingIndex set) override;
        void update_texture(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, uint_t layer,
                            const byte* data);
        void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap,
                               const byte* data) override;
        bool can_use_color_as_color_attachment() const;
        bool is_depth_stencil_image() const;
        vk::Offset2D get_mip_size(MipMapLevel level) const;
        vk::ImageView get_image_view(const vk::ImageSubresourceRange& range);


        ~VulkanTexture();
    };
}// namespace Engine
