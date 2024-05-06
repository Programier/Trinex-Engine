#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>


namespace Engine
{
    struct VulkanTexture : RHI_Texture {
    private:
        const Engine::Texture* m_engine_texture;
        vk::DeviceMemory m_image_memory;

        vk::Image m_image;
        vk::ImageView m_image_view;
        vk::ImageLayout m_layout;
        vk::Format m_vulkan_format;
        vk::ComponentMapping m_swizzle;

    public:
        vk::DeviceMemory memory() const;
        vk::Image image() const;
        vk::ImageView image_view() const;
        vk::ImageLayout layout() const;
        vk::Format format() const;
        vk::ComponentMapping swizzle() const;
        Vector2D size() const;
        uint_t layer_count() const;
        MipMapLevel mipmap_count();
        MipMapLevel base_mipmap();
        vk::ImageAspectFlags aspect(bool use_for_shader_attachment = false) const;

        VulkanTexture& create(const Texture* texture, const byte* data, size_t size);
        VulkanTexture& destroy();


        vk::ImageViewType view_type() const;

        void generate_mipmap() override;
        void bind(BindLocation location) override;
        void bind_combined(RHI_Sampler* sampler, BindLocation location) override;
        void update_texture(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, uint_t layer, const byte* data,
                            size_t data_size);
        void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data,
                               size_t data_size) override;
        bool is_color_image() const;
        bool is_depth_stencil_image() const;
        static bool is_depth_stencil_image(ColorFormat);

        vk::Offset2D get_mip_size(MipMapLevel level) const;
        vk::ImageView create_image_view(const vk::ImageSubresourceRange& range);
        vk::ImageLayout change_layout(vk::ImageLayout new_layout);
        vk::ImageLayout change_layout(vk::ImageLayout new_layout, vk::CommandBuffer& cmd);

        ~VulkanTexture();
    };
}// namespace Engine
