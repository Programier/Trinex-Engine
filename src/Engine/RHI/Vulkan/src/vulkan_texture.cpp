#include <Core/default_resources.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <imgui_impl_vulkan.h>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>

namespace Engine
{

    vk::DeviceMemory VulkanTexture::memory() const
    {
        return m_image_memory;
    }

    vk::Image VulkanTexture::image() const
    {
        return m_image;
    }

    vk::ImageView VulkanTexture::image_view() const
    {
        return m_image_view;
    }

    vk::ImageLayout VulkanTexture::layout() const
    {
        return m_layout;
    }

    vk::Format VulkanTexture::format() const
    {
        return m_vulkan_format;
    }

    vk::ComponentMapping VulkanTexture::swizzle() const
    {
        return m_swizzle;
    }

    Vector2D VulkanTexture::size() const
    {
        return m_engine_texture->size;
    }

    MipMapLevel VulkanTexture::mipmap_count()
    {
        return m_engine_texture->mipmap_count;
    }

    MipMapLevel VulkanTexture::base_mipmap()
    {
        return m_engine_texture->base_mip_level;
    }

    VulkanTexture& VulkanTexture::create(const Texture* texture, const byte* data, size_t data_size)
    {
        m_layout         = vk::ImageLayout::eUndefined;
        m_engine_texture = texture;
        m_vulkan_format  = parse_engine_format(m_engine_texture->format);

        static vk::ImageCreateFlagBits default_flags = {};

        vk::ImageUsageFlags m_usage_flags =
                vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        vk::MemoryPropertyFlags m_memory_flags = vk::MemoryPropertyFlagBits::eHostCoherent;


        if (texture->is_render_target_texture())
        {
            if (is_depth_stencil_image())
            {
                m_usage_flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            }
            else if (is_color_image())
            {
                m_usage_flags |= vk::ImageUsageFlagBits::eColorAttachment;
            }
        }

        vk::ImageTiling tiling = vk::ImageTiling::eOptimal;

        API->create_image(this, tiling,
                          m_engine_texture->type() == TextureType::Texture2D ? default_flags
                                                                             : vk::ImageCreateFlagBits::eCubeCompatible,
                          m_usage_flags, m_memory_flags, m_image, m_image_memory, layer_count());


        // Creating image view
        m_swizzle = vk::ComponentMapping(get_type(texture->swizzle_r), get_type(texture->swizzle_g), get_type(texture->swizzle_b),
                                         get_type(texture->swizzle_a));
        m_image_view = create_image_view(
                vk::ImageSubresourceRange(aspect(true), m_engine_texture->base_mip_level,
                                          m_engine_texture->mipmap_count - m_engine_texture->base_mip_level, 0, layer_count()));

        change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);

        if (data)
        {
            if (texture->type() == TextureType::Texture2D)
            {
                update_texture_2D(texture->size, {0, 0}, 0, data, data_size);
            }
        }

        if (m_engine_texture->mipmap_count > 1)
            generate_mipmap();
        return *this;
    }

    void VulkanTexture::update_texture(const Size2D& size, const Offset2D& offset, MipMapLevel level, uint_t layer,
                                       const byte* data, size_t data_size)
    {
        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;

        vk::DeviceSize buffer_size = data_size;

        API->create_buffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc,
                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging_buffer,
                           staging_buffer_memory);

        void* vulkan_data = API->m_device.mapMemory(staging_buffer_memory, 0, buffer_size);
        std::memcpy(vulkan_data, data, buffer_size);
        API->m_device.unmapMemory(staging_buffer_memory);

        vk::ImageMemoryBarrier barrier;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.oldLayout           = m_layout;
        barrier.newLayout           = vk::ImageLayout::eTransferDstOptimal;
        barrier.image               = m_image;
        barrier.subresourceRange    = vk::ImageSubresourceRange(aspect(), level, 1, layer, 1);


        Barrier::transition_image_layout(barrier);
        auto command_buffer = API->begin_single_time_command_buffer();

        vk::BufferImageCopy region(0, 0, 0, vk::ImageSubresourceLayers(aspect(), level, layer, 1),
                                   vk::Offset3D(static_cast<uint_t>(offset.x), static_cast<uint_t>(offset.y), 0),
                                   vk::Extent3D(static_cast<uint_t>(size.x), static_cast<uint_t>(size.y), 1));

        command_buffer.copyBufferToImage(staging_buffer, m_image, vk::ImageLayout::eTransferDstOptimal, region);

        API->end_single_time_command_buffer(command_buffer);

        API->m_device.freeMemory(staging_buffer_memory, nullptr);
        API->m_device.destroyBuffer(staging_buffer);


        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = m_layout;
        Barrier::transition_image_layout(barrier);
    }

    void VulkanTexture::update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data,
                                          size_t data_size)
    {
        update_texture(size, offset, mipmap, 0, data, data_size);
    }


    void VulkanTexture::bind(BindLocation location)
    {
        if (API->m_state->m_pipeline)
        {
            API->m_state->m_pipeline->bind_texture(this, location);
        }
    }

    void VulkanTexture::bind_combined(RHI_Sampler* sampler, BindLocation location)
    {
        if (API->m_state->m_pipeline)
        {
            trinex_always_check(sampler, "Sampler can't be null!");
            API->m_state->m_pipeline->bind_texture_combined(this, reinterpret_cast<VulkanSampler*>(sampler), location);
        }
    }

    VulkanTexture& VulkanTexture::destroy()
    {
        DESTROY_CALL(destroyImage, m_image);
        DESTROY_CALL(freeMemory, m_image_memory);
        DESTROY_CALL(destroyImageView, m_image_view);
        return *this;
    }

    uint_t VulkanTexture::layer_count() const
    {
        return m_engine_texture->type() == TextureType::Texture2D ? 1 : 6;
    }

    vk::ImageViewType VulkanTexture::view_type() const
    {
        return m_engine_texture->type() == TextureType::Texture2D ? vk::ImageViewType::e2D : vk::ImageViewType::eCube;
    }

    vk::ImageAspectFlags VulkanTexture::aspect(bool use_for_shader_attachment) const
    {
        if (is_color_image())
        {
            return vk::ImageAspectFlagBits::eColor;
        }
        else if (is_in<ColorFormat::DepthStencil>(m_engine_texture->format))
        {
            if (use_for_shader_attachment)
            {
                return vk::ImageAspectFlagBits::eDepth;
            }
            else
            {
                return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            }
        }

        return vk::ImageAspectFlagBits::eDepth;
    }

    bool VulkanTexture::is_color_image() const
    {
        return is_in<ColorFormat::R8G8B8A8, ColorFormat::FloatRGBA>(m_engine_texture->format);
    }

    bool VulkanTexture::is_depth_stencil_image() const
    {
        return is_depth_stencil_image(m_engine_texture->format);
    }

    bool VulkanTexture::is_depth_stencil_image(ColorFormat format)
    {
        return is_in<ColorFormat::DepthStencil, ColorFormat::D32F, ColorFormat::ShadowDepth, ColorFormat::FilteredShadowDepth>(
                format);
    }

    void VulkanTexture::generate_mipmap()
    {
        if (!m_image)
            return;

        vk::FormatProperties format_properties = API->m_physical_device.getFormatProperties(m_vulkan_format);

        if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        {
            throw std::runtime_error("VulkanAPI: Texture image format does not support linear blitting!");
        }

        if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst))
        {
            throw std::runtime_error("VulkanAPI: Texture image format does not support linear blitting!");
        }

        vk::CommandBuffer command_buffer = API->begin_single_time_command_buffer();

        auto subresource = vk::ImageSubresourceRange(aspect(), base_mipmap(), m_engine_texture->mipmap_count - base_mipmap(), 0,
                                                     layer_count());
        subresource.levelCount = 1;
        subresource.layerCount = 1;

        for (uint_t layer = 0, count = layer_count(); layer < count; layer++)
        {
            subresource.baseArrayLayer = layer;

            vk::ImageMemoryBarrier barrier({}, {}, {}, {}, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, m_image,
                                           subresource);
            vk::ImageMemoryBarrier barrier2 = barrier;

            for (MipMapLevel i = 1; i < m_engine_texture->base_mip_level; i++)
            {
                if (i >= m_engine_texture->base_mip_level)
                {
                    barrier.subresourceRange.baseMipLevel = i == m_engine_texture->base_mip_level ? 0 : i - 1;
                    barrier.oldLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
                    barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
                    barrier.srcAccessMask                 = vk::AccessFlagBits::eShaderRead;
                    barrier.dstAccessMask                 = vk::AccessFlagBits::eTransferRead;

                    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                                                   vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

                    barrier2.subresourceRange.baseMipLevel = i;
                    barrier2.oldLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
                    barrier2.newLayout                     = vk::ImageLayout::eTransferDstOptimal;
                    barrier2.srcAccessMask                 = vk::AccessFlagBits::eShaderRead;
                    barrier2.dstAccessMask                 = vk::AccessFlagBits::eTransferWrite;

                    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                                                   vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier2);

                    vk::ImageSubresourceLayers src_image_subresource_layers(
                            aspect(), (i == m_engine_texture->base_mip_level ? 0 : i - 1), layer, 1);

                    vk::ImageSubresourceLayers dst_image_subresource_layers(aspect(), i, layer, 1);

                    auto base_mip_size    = get_mip_size(barrier.subresourceRange.baseMipLevel);
                    auto current_mip_size = get_mip_size(i);

                    const Array<vk::Offset3D, 2> src_offsets = {vk::Offset3D(0, 0, 0),
                                                                vk::Offset3D(base_mip_size.x, base_mip_size.y, 1)};

                    const Array<vk::Offset3D, 2> dst_offsets = {vk::Offset3D(0, 0, 0),
                                                                vk::Offset3D(current_mip_size.x, current_mip_size.y, 1)};

                    vk::ImageBlit blit(src_image_subresource_layers, src_offsets, dst_image_subresource_layers, dst_offsets);

                    command_buffer.blitImage(m_image, vk::ImageLayout::eTransferSrcOptimal, m_image,
                                             vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

                    barrier2.oldLayout     = vk::ImageLayout::eTransferDstOptimal;
                    barrier2.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
                    barrier2.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                    barrier2.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                   vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier2);

                    barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
                    barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
                    barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                   vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
                }
            }
        }
        API->end_single_time_command_buffer(command_buffer);
    }

    vk::Offset2D VulkanTexture::get_mip_size(MipMapLevel level) const
    {
        uint_t width  = glm::max(static_cast<uint_t>(m_engine_texture->size.x) >> level, 1U);
        uint_t height = glm::max(static_cast<uint_t>(m_engine_texture->size.y) >> level, 1U);
        return vk::Offset2D(width, height);
    }

    vk::ImageView VulkanTexture::create_image_view(const vk::ImageSubresourceRange& range)
    {
        vk::ImageViewCreateInfo view_info({}, m_image, view_type(), m_vulkan_format, m_swizzle, range);
        return API->m_device.createImageView(view_info);
    }

    vk::ImageLayout VulkanTexture::change_layout(vk::ImageLayout new_layout)
    {
        if (layout() != new_layout)
        {
            auto base_mip = base_mipmap();

            vk::ImageMemoryBarrier barrier;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.oldLayout           = m_layout;
            barrier.newLayout           = new_layout;
            barrier.image               = m_image;

            barrier.subresourceRange = vk::ImageSubresourceRange(aspect(), base_mip, mipmap_count() - base_mip, 0, layer_count());
            m_layout                 = barrier.newLayout;

            Barrier::transition_image_layout(barrier);

            m_layout = new_layout;
        }
        return new_layout;
    }


    VulkanTexture::~VulkanTexture()
    {
        destroy();
    }

    RHI_Texture* VulkanAPI::create_texture(const Texture* texture, const byte* data, size_t size)
    {
        return &(new VulkanTexture())->create(texture, data, size);
    }
}// namespace Engine

VkImageView trinex_vulkan_image_view(Engine::Texture2D* texture)
{
    return (texture ? texture : Engine::DefaultResources::default_texture)->rhi_object<Engine::VulkanTexture>()->image_view();
}
