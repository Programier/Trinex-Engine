#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/memory.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <imgui_impl_vulkan.h>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
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


    vk::ComponentMapping VulkanTexture::swizzle() const
    {
        return m_swizzle;
    }

    MipMapLevel VulkanTexture::base_mipmap()
    {
        return 0;
    }

    void VulkanTexture::clear_color(const Color& color)
    {}

    void VulkanTexture::clear_depth_stencil(float depth, byte stencil)
    {}

    VulkanTexture& VulkanTexture::create(const Texture* texture)
    {
        m_layout = vk::ImageLayout::eUndefined;

        static vk::ImageCreateFlagBits default_flags = {};

        vk::ImageUsageFlags m_usage_flags      = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        vk::MemoryPropertyFlags m_memory_flags = vk::MemoryPropertyFlagBits::eHostCoherent;


        if (texture->class_instance()->is_a<RenderSurface>())
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
                          texture->type() == TextureType::Texture2D ? default_flags : vk::ImageCreateFlagBits::eCubeCompatible,
                          m_usage_flags, m_memory_flags, m_image, m_image_memory, layer_count());

        // Creating image view
        m_swizzle = vk::ComponentMapping(get_type(texture->swizzle_r), get_type(texture->swizzle_g), get_type(texture->swizzle_b),
                                         get_type(texture->swizzle_a));
        m_image_view = create_image_view(vk::ImageSubresourceRange(aspect(true), 0, mipmap_count(), 0, layer_count()));
        change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);

        return *this;
    }

    void VulkanTexture::update_texture(const Size2D& size, MipMapLevel level, uint_t layer, const byte* data, size_t data_size)
    {
        if (data == nullptr || data_size == 0)
            return;

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

        vk::BufferImageCopy region(0, 0, 0, vk::ImageSubresourceLayers(aspect(), level, layer, 1), vk::Offset3D(0, 0, 0),
                                   vk::Extent3D(static_cast<uint_t>(size.x), static_cast<uint_t>(size.y), 1));

        command_buffer.copyBufferToImage(staging_buffer, m_image, vk::ImageLayout::eTransferDstOptimal, region);

        API->end_single_time_command_buffer(command_buffer);

        API->m_device.freeMemory(staging_buffer_memory, nullptr);
        API->m_device.destroyBuffer(staging_buffer);


        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = m_layout;
        Barrier::transition_image_layout(barrier);
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

    vk::ImageAspectFlags VulkanTexture::aspect(bool use_for_shader_attachment) const
    {
        if (is_color_image())
        {
            return vk::ImageAspectFlagBits::eColor;
        }
        else if (is_in<parse_engine_format(ColorFormat::DepthStencil)>(format()))
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
        return is_in<ColorFormat::FloatR, ColorFormat::FloatRGBA, ColorFormat::R8, ColorFormat::R8G8B8A8, ColorFormat::BC1,
                     ColorFormat::BC2, ColorFormat::BC3>(engine_format());
    }

    bool VulkanTexture::is_render_target_color_image() const
    {
        return is_in<ColorFormat::R8G8B8A8, ColorFormat::FloatRGBA>(engine_format());
    }

    bool VulkanTexture::is_depth_stencil_image() const
    {
        return is_depth_stencil_image(engine_format());
    }

    bool VulkanTexture::is_depth_stencil_image(ColorFormat format)
    {
        return is_in<ColorFormat::DepthStencil, ColorFormat::D32F, ColorFormat::ShadowDepth, ColorFormat::FilteredShadowDepth>(
                format);
    }

    vk::ImageView VulkanTexture::create_image_view(const vk::ImageSubresourceRange& range)
    {
        vk::ImageViewCreateInfo view_info({}, m_image, view_type(), format(), m_swizzle, range);
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

    vk::ImageLayout VulkanTexture::change_layout(vk::ImageLayout new_layout, vk::CommandBuffer& cmd)
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

            Barrier::transition_image_layout(cmd, barrier);

            m_layout = new_layout;
        }
        return new_layout;
    }


    VulkanTexture::~VulkanTexture()
    {
        destroy();
    }

    VulkanTexture2D& VulkanTexture2D::create(const Texture2D* texture)
    {
        m_texture = texture;
        VulkanTexture::create(texture);

        for (MipMapLevel i = 0, count = texture->mipmap_count(); i < count; ++i)
        {
            if (auto mip = texture->mip(i))
            {
                update_texture(mip->size, i, 0, mip->data.data(), mip->data.size());
            }
        }
        return *this;
    }

    uint_t VulkanTexture2D::layer_count() const
    {
        return 1;
    }

    vk::ImageCreateFlagBits VulkanTexture2D::create_flags() const
    {
        return {};
    }

    vk::ImageViewType VulkanTexture2D::view_type() const
    {
        return vk::ImageViewType::e2D;
    }

    Size2D VulkanTexture2D::size() const
    {
        return m_texture->size();
    }

    MipMapLevel VulkanTexture2D::mipmap_count() const
    {
        return m_texture->mipmap_count();
    }

    vk::Format VulkanTexture2D::format() const
    {
        return parse_engine_format(engine_format());
    }

    ColorFormat VulkanTexture2D::engine_format() const
    {
        return m_texture->format();
    }

    VulkanSurface& VulkanSurface::create(const Texture2D* texture)
    {
        m_size = texture->size();
        VulkanTexture2D::create(texture);
        return *this;
    }

    Size2D VulkanSurface::size() const
    {
        return m_size;
    }

    void VulkanSurface::clear_color(const Color& color)
    {
        if (is_color_image())
        {
        }
    }

    void VulkanSurface::clear_depth_stencil(float depth, byte stencil)
    {
        if (is_depth_stencil_image())
        {
            auto current_layout = layout();
            auto& cmd           = API->current_command_buffer();
            change_layout(vk::ImageLayout::eTransferDstOptimal, cmd);

            vk::ClearDepthStencilValue value;
            value.setDepth(depth).setStencil(stencil);
            vk::ImageSubresourceRange range;
            range.setAspectMask(aspect(false))
                    .setBaseArrayLayer(0)
                    .setBaseMipLevel(0)
                    .setLayerCount(layer_count())
                    .setLevelCount(mipmap_count());


            API->current_command_buffer().clearDepthStencilImage(image(), layout(), value, range);
            change_layout(current_layout, cmd);
        }
    }

    VulkanSurface::~VulkanSurface()
    {
        while (!m_render_targets.empty())
        {
            VulkanRenderTarget* rt = *m_render_targets.begin();
            delete rt;
        }
    }

    RHI_Texture* VulkanAPI::create_texture_2d(const Texture2D* texture)
    {
        if (texture->class_instance()->is_a<RenderSurface>())
        {
            return &(new VulkanSurface())->create(texture);
        }

        return &(new VulkanTexture2D())->create(texture);
    }
}// namespace Engine

VkImageView trinex_vulkan_image_view(Engine::Texture2D* texture)
{
    return (texture ? texture : Engine::DefaultResources::default_texture)->rhi_object<Engine::VulkanTexture>()->image_view();
}
