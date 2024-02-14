#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <imgui_impl_vulkan.h>
#include <vulkan_api.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_transition_image_layout.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
    VulkanTexture& VulkanTexture::create(const Texture* texture, const byte* data)
    {
        destroy();

        _M_engine_texture = texture;
        _M_vulkan_format  = parse_engine_format(_M_engine_texture->format);

        static vk::ImageCreateFlagBits default_flags = {};

        vk::ImageUsageFlags _M_usage_flags;
        vk::MemoryPropertyFlags _M_memory_flags;

        ColorFormatInfo format_info     = ColorFormatInfo::info_of(texture->format);
        ColorFormatAspect format_aspect = format_info.aspect();

        if (format_aspect == ColorFormatAspect::Depth || format_aspect == ColorFormatAspect::Stencil ||
            format_aspect == ColorFormatAspect::DepthStencil)
        {
            _M_usage_flags = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc |
                             vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
            _M_memory_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        }
        else
        {
            _M_usage_flags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                             vk::ImageUsageFlagBits::eSampled;

            if (texture->is_render_target_texture() && can_use_color_as_color_attachment())
            {
                _M_usage_flags |= vk::ImageUsageFlagBits::eColorAttachment;
            }

            _M_memory_flags = vk::MemoryPropertyFlagBits::eHostCoherent;
        }

        vk::ImageTiling tiling = texture->is_render_target_texture() ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear;

        API->create_image(this, tiling,
                          _M_engine_texture->type() == TextureType::Texture2D ? default_flags
                                                                              : vk::ImageCreateFlagBits::eCubeCompatible,
                          _M_usage_flags, _M_memory_flags, _M_image, _M_image_memory, layer_count());


        // Creating image view
        _M_swizzle = vk::ComponentMapping(get_type(texture->swizzle_r), get_type(texture->swizzle_g),
                                          get_type(texture->swizzle_b), get_type(texture->swizzle_a));
        vk::ImageViewCreateInfo view_info({}, _M_image, view_type(), _M_vulkan_format, _M_swizzle,
                                          subresource_range(texture->base_mip_level));
        _M_image_view = API->_M_device.createImageView(view_info);


        {
            TransitionImageLayout transition;
            transition.image        = &_M_image;
            transition.base_mip     = 0;
            transition.mip_count    = _M_engine_texture->mipmap_count;
            transition.base_layer   = 0;
            transition.layer_count  = layer_count();
            transition.aspect_flags = aspect();

            transition.old_layout = vk::ImageLayout::eUndefined;
            transition.new_layout = vk::ImageLayout::eShaderReadOnlyOptimal;


            transition.execute(vk::PipelineStageFlagBits::eTopOfPipe, vk::AccessFlagBits::eNone,
                               vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader |
                                       vk::PipelineStageFlagBits::eComputeShader,
                               vk::AccessFlagBits::eShaderRead);
        }

        if (data)
        {
            if (texture->type() == TextureType::Texture2D)
            {
                update_texture_2D(texture->size, {0, 0}, 0, data);
            }
        }
        return *this;
    }

    void VulkanTexture::update_texture(const Size2D& size, const Offset2D& offset, MipMapLevel level, uint_t layer,
                                       const byte* data)
    {
        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;

        vk::DeviceSize buffer_size =
                static_cast<vk::DeviceSize>(size.x) * static_cast<vk::DeviceSize>(size.y) * pixel_type_size();


        API->create_buffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc,
                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging_buffer,
                           staging_buffer_memory);

        void* vulkan_data = API->_M_device.mapMemory(staging_buffer_memory, 0, buffer_size);
        std::memcpy(vulkan_data, data, buffer_size);
        API->_M_device.unmapMemory(staging_buffer_memory);


        TransitionImageLayout transition;
        transition.image       = &_M_image;
        transition.old_layout  = vk::ImageLayout::eShaderReadOnlyOptimal;
        transition.new_layout  = vk::ImageLayout::eTransferDstOptimal;
        transition.base_mip    = level;
        transition.mip_count   = 1;
        transition.base_layer  = layer;
        transition.layer_count = 1;

        transition.execute(vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead,
                           vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite);

        auto command_buffer = API->begin_single_time_command_buffer();

        vk::BufferImageCopy region(0, 0, 0, vk::ImageSubresourceLayers(aspect(), level, layer, 1),
                                   vk::Offset3D(static_cast<uint_t>(offset.x), static_cast<uint_t>(offset.y), 0),
                                   vk::Extent3D(static_cast<uint_t>(size.x), static_cast<uint_t>(size.y), 1));

        command_buffer.copyBufferToImage(staging_buffer, _M_image, vk::ImageLayout::eTransferDstOptimal, region);

        API->end_single_time_command_buffer(command_buffer);

        API->_M_device.freeMemory(staging_buffer_memory, nullptr);
        API->_M_device.destroyBuffer(staging_buffer);

        std::swap(transition.old_layout, transition.new_layout);
        transition.execute(vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite,
                           vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead);
    }

    void VulkanTexture::update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data)
    {
        update_texture(size, offset, mipmap, 0, data);
    }


    void VulkanTexture::bind(BindLocation location)
    {
        if (API->_M_state->_M_pipeline)
        {
            API->_M_state->_M_pipeline->bind_texture(this, location);
        }
    }

    void VulkanTexture::bind_combined(RHI_Sampler* sampler, BindLocation location)
    {
        if (API->_M_state->_M_pipeline)
        {
            API->_M_state->_M_pipeline->bind_combined_sampler(reinterpret_cast<VulkanSampler*>(sampler), this, location);
        }
    }

    VulkanTexture& VulkanTexture::destroy()
    {
        DESTROY_CALL(destroyImage, _M_image);
        DESTROY_CALL(freeMemory, _M_image_memory);
        DESTROY_CALL(destroyImageView, _M_image_view);
        return *this;
    }

    vk::ImageSubresourceRange VulkanTexture::subresource_range(MipMapLevel base)
    {
        return vk::ImageSubresourceRange(aspect(), base, _M_engine_texture->mipmap_count - base, 0, layer_count());
    }

    uint_t VulkanTexture::layer_count() const
    {
        return _M_engine_texture->type() == TextureType::Texture2D ? 1 : 6;
    }

    vk::ImageViewType VulkanTexture::view_type() const
    {
        return _M_engine_texture->type() == TextureType::Texture2D ? vk::ImageViewType::e2D : vk::ImageViewType::eCube;
    }

    vk::ImageAspectFlags VulkanTexture::aspect() const
    {
        return get_type(ColorFormatInfo::info_of(_M_engine_texture->format).aspect());
    }

    bool VulkanTexture::can_use_color_as_color_attachment() const
    {
        return ColorFormatInfo::info_of(_M_engine_texture->format).components() == 4;
    }

    uint_t VulkanTexture::pixel_type_size() const
    {
        ColorFormatInfo color_info = ColorFormatInfo::info_of(_M_engine_texture->format);
        return static_cast<uint_t>(color_info.component_size()) * static_cast<uint_t>(color_info.components());
    }

    bool VulkanTexture::is_depth_stencil_image() const
    {
        ColorFormatAspect texture_aspect = ColorFormatInfo::info_of(_M_engine_texture->format).aspect();
        return texture_aspect == ColorFormatAspect::Depth || texture_aspect == ColorFormatAspect::Stencil ||
               texture_aspect == ColorFormatAspect::DepthStencil;
    }

    void VulkanTexture::generate_mipmap()
    {
        if (!_M_image)
            return;

        vk::FormatProperties format_properties = API->_M_physical_device.getFormatProperties(_M_vulkan_format);

        if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        {
            throw std::runtime_error("VulkanAPI: Texture image format does not support linear blitting!");
        }

        if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst))
        {
            throw std::runtime_error("VulkanAPI: Texture image format does not support linear blitting!");
        }

        vk::CommandBuffer command_buffer = API->begin_single_time_command_buffer();

        auto subresource       = subresource_range(_M_engine_texture->base_mip_level);
        subresource.levelCount = 1;
        subresource.layerCount = 1;

        for (uint_t layer = 0, count = layer_count(); layer < count; layer++)
        {
            subresource.baseArrayLayer = layer;

            vk::ImageMemoryBarrier barrier({}, {}, {}, {}, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, _M_image,
                                           subresource);
            vk::ImageMemoryBarrier barrier2 = barrier;

            for (MipMapLevel i = 1; i < _M_engine_texture->base_mip_level; i++)
            {
                if (i >= _M_engine_texture->base_mip_level)
                {
                    barrier.subresourceRange.baseMipLevel = i == _M_engine_texture->base_mip_level ? 0 : i - 1;
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
                            aspect(), (i == _M_engine_texture->base_mip_level ? 0 : i - 1), layer, 1);

                    vk::ImageSubresourceLayers dst_image_subresource_layers(aspect(), i, layer, 1);

                    auto base_mip_size    = get_mip_size(barrier.subresourceRange.baseMipLevel);
                    auto current_mip_size = get_mip_size(i);

                    const Array<vk::Offset3D, 2> src_offsets = {vk::Offset3D(0, 0, 0),
                                                                vk::Offset3D(base_mip_size.x, base_mip_size.y, 1)};

                    const Array<vk::Offset3D, 2> dst_offsets = {vk::Offset3D(0, 0, 0),
                                                                vk::Offset3D(current_mip_size.x, current_mip_size.y, 1)};

                    vk::ImageBlit blit(src_image_subresource_layers, src_offsets, dst_image_subresource_layers, dst_offsets);

                    command_buffer.blitImage(_M_image, vk::ImageLayout::eTransferSrcOptimal, _M_image,
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
        uint_t width  = glm::max(static_cast<uint_t>(_M_engine_texture->size.x) >> level, 1U);
        uint_t height = glm::max(static_cast<uint_t>(_M_engine_texture->size.y) >> level, 1U);
        return vk::Offset2D(width, height);
    }

    vk::ImageView VulkanTexture::get_image_view(const vk::ImageSubresourceRange& range)
    {
        vk::ImageViewCreateInfo view_info({}, _M_image, view_type(), _M_vulkan_format, _M_swizzle, range);
        return API->_M_device.createImageView(view_info);
    }


    VulkanTexture::~VulkanTexture()
    {
        destroy();
    }

    RHI_Texture* VulkanAPI::create_texture(const Texture* texture, const byte* data)
    {
        return &(new VulkanTexture())->create(texture, data);
    }

    ColorFormatFeatures VulkanAPI::color_format_features(ColorFormat format)
    {
        vk::Format vulkan_format        = parse_engine_format(format);
        vk::FormatProperties properties = _M_physical_device.getFormatProperties(vulkan_format);
        ColorFormatFeatures out_features;

        if (!properties.linearTilingFeatures && !properties.optimalTilingFeatures)
        {
            out_features.is_supported = false;
            return out_features;
        }

        out_features.is_supported = true;

        out_features.support_color_attachment =
                (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment) ==
                vk::FormatFeatureFlagBits::eColorAttachment;

        out_features.support_depth_stencil =
                (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) ==
                vk::FormatFeatureFlagBits::eDepthStencilAttachment;

        return out_features;
    }
}// namespace Engine
