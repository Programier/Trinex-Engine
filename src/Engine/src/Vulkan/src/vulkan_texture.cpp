#include <vulkan_api.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_transition_image_layout.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
    using ComponentsArray = vk::Format[8];
    using ColorFormat     = ComponentsArray[6];

    static ColorFormat color_formats = {
            {vk::Format::eR8G8B8Srgb, vk::Format::eR32G32B32Sfloat, vk::Format::eUndefined, vk::Format::eUndefined,
             vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eR16G16B16Sfloat},

            {vk::Format::eR8G8B8A8Srgb, vk::Format::eR32G32B32A32Sfloat, vk::Format::eUndefined, vk::Format::eUndefined,
             vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eR16G16B16A16Sfloat},

            {vk::Format::eR8Uint, vk::Format::eR32Sfloat, vk::Format::eUndefined, vk::Format::eUndefined,
             vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eR16Sfloat},

            {vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eD16Unorm, vk::Format::eUndefined,
             vk::Format::eUndefined, vk::Format::eD32Sfloat, vk::Format::eUndefined, vk::Format::eUndefined},

            {vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eS8Uint,
             vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eUndefined},

            {vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eUndefined, vk::Format::eUndefined,
             vk::Format::eD32SfloatS8Uint, vk::Format::eUndefined, vk::Format::eD24UnormS8Uint, vk::Format::eUndefined},
    };

    vk::Format VulkanTexture::parse_format(PixelType type, PixelComponentType component)
    {
        EnumerateType pt  = static_cast<EnumerateType>(type);
        EnumerateType pct = static_cast<EnumerateType>(component);

        if (pt > 5 || pct > 7)
            throw EngineException("Incorect format!");

        vk::Format result = color_formats[pt][pct];
        if (result == vk::Format::eUndefined)
            throw EngineException("Incorect format!");
        return result;
    }

    VulkanTextureState& VulkanTextureState::init(const TextureCreateInfo& info)
    {
        size.setWidth(static_cast<uint32_t>(info.size.x)).setHeight(static_cast<uint32_t>(info.size.y));
        mipmap_count             = glm::max(static_cast<MipMapLevel>(1), info.mipmap_count);
        format                   = VulkanTexture::parse_format(info.pixel_type, info.pixel_component_type);
        min_filter               = get_type(info.min_filter);
        mag_filter               = get_type(info.mag_filter);
        sampler_mipmap_mode      = get_type(info.mipmap_mode);
        wrap_s                   = get_type(info.wrap_s);
        wrap_r                   = get_type(info.wrap_r);
        wrap_t                   = get_type(info.wrap_t);
        mip_lod_bias             = info.mip_lod_bias;
        anisotropy_enable        = info.anisotropy > 1.0f;
        anisotropy               = glm::max(glm::min(API->max_anisotropic_filtering(), info.anisotropy), 1.0f);
        compare_enable           = info.compare_mode == CompareMode::RefToTexture;
        compare_func             = get_type(info.compare_func);
        min_lod                  = info.min_lod;
        max_lod                  = info.max_lod;
        unnormalized_coordinates = 0;
        base_mip_level           = info.base_mip_level;
        swizzle = vk::ComponentMapping(get_type(info.swizzle.R), get_type(info.swizzle.G), get_type(info.swizzle.B),
                                       get_type(info.swizzle.A));
        return *this;
    }

    VulkanTexture::VulkanTexture()
    {
        _M_image_aspect     = vk::ImageAspectFlags();
        _M_instance_address = this;
    }


    VulkanTexture& VulkanTexture::init(const TextureCreateInfo& info, TextureType type)
    {
        _M_image_aspect = _M_image_aspects[static_cast<EnumerateType>(info.pixel_component_type)];
        state.init(info);

        if (type == TextureType::Texture2D)
        {
            _M_image_type = vk::ImageViewType::e2D;
        }
        else
        {
            _M_image_type  = vk::ImageViewType::eCube;
            _M_layer_count = 6;
        }

        create_image().create_image_view().create_texture_sampler();

        TransitionImageLayout transition;
        transition.image        = &_M_image;
        transition.base_mip     = 0;
        transition.mip_count    = state.mipmap_count;
        transition.base_layer   = 0;
        transition.layer_count  = _M_layer_count;
        transition.aspect_flags = _M_image_aspect;

        transition.old_layout = vk::ImageLayout::eUndefined;
        transition.new_layout = vk::ImageLayout::eShaderReadOnlyOptimal;


        transition.execute(vk::PipelineStageFlagBits::eTopOfPipe, vk::AccessFlagBits::eNone,
                           vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead);
        return *this;
    }


#define SIZE(component) static_cast<uint_t>(state.base_size.component)
#define HAS_ASPECT_FLAG(f)                                                                                             \
    ((_M_image_aspect & vk::ImageAspectFlagBits::f) == static_cast<vk::ImageAspectFlags>(vk::ImageAspectFlagBits::f))

    VulkanTexture& VulkanTexture::create_image()
    {
        static vk::ImageCreateFlagBits default_flags = {};

        vk::ImageUsageFlags _M_usage_flags;
        vk::MemoryPropertyFlags _M_memory_flags;

        if (HAS_ASPECT_FLAG(eDepth) || HAS_ASPECT_FLAG(eStencil))
        {
            _M_usage_flags = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc |
                             vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
            _M_memory_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        }
        else
        {
            _M_usage_flags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                             vk::ImageUsageFlagBits::eSampled;

            if (can_use_color_as_color_attachment())
            {
                _M_usage_flags |= vk::ImageUsageFlagBits::eColorAttachment;
            }

            _M_memory_flags = vk::MemoryPropertyFlagBits::eHostCoherent;
        }

        API->create_image(this, vk::ImageTiling::eOptimal,
                          _M_layer_count == 1 ? default_flags : vk::ImageCreateFlagBits::eCubeCompatible,
                          _M_usage_flags, _M_memory_flags, _M_image, _M_image_memory, _M_layer_count);
        return *this;
    }


    vk::Format VulkanTexture::format()
    {
        return state.format;
    }

    vk::ImageSubresourceRange VulkanTexture::subresource_range(MipMapLevel base)
    {
        return vk::ImageSubresourceRange(_M_image_aspect, base, state.mipmap_count - base, 0, _M_layer_count);
    }

    uint_t VulkanTexture::pixel_type_size()
    {
        switch (state.format)
        {
            case vk::Format::eR32G32B32Sfloat:
                return 12;

            case vk::Format::eR32G32B32A32Sfloat:
                return 16;

            case vk::Format::eR32Sfloat:
            case vk::Format::eD32Sfloat:
            case vk::Format::eR8G8B8A8Srgb:
            case vk::Format::eD24UnormS8Uint:
                return 4;

            case vk::Format::eR8G8B8Srgb:
            case vk::Format::eD16UnormS8Uint:
                return 3;

            case vk::Format::eR8Srgb:
                return 1;

            case vk::Format::eD16Unorm:
                return 2;

            case vk::Format::eS8Uint:
                return 1;

            case vk::Format::eD32SfloatS8Uint:
                return 5;
            default:
                return 0;
        }
    }

    vk::ImageView VulkanTexture::get_image_view(const vk::ImageSubresourceRange& range)
    {
        vk::ImageViewCreateInfo view_info({}, _M_image, _M_image_type, format(), state.swizzle, range);
        return API->_M_device.createImageView(view_info);
    }

    VulkanTexture& VulkanTexture::create_image_view()
    {
        API->wait_idle();
        DESTROY_CALL(destroyImageView, _M_image_view);

        _M_image_view = get_image_view(subresource_range(state.base_mip_level));
        return *this;
    }

    VulkanTexture& VulkanTexture::create_texture_sampler()
    {
        API->wait_idle();
        DESTROY_CALL(destroySampler, _M_texture_sampler);

        vk::SamplerCreateInfo sampler_info({}, state.mag_filter, state.min_filter, state.sampler_mipmap_mode,
                                           state.wrap_s, state.wrap_t, state.wrap_r, state.mip_lod_bias,
                                           static_cast<vk::Bool32>(state.anisotropy > 1.0) && state.anisotropy_enable,
                                           state.anisotropy, state.compare_enable, state.compare_func, state.min_lod,
                                           state.max_lod, vk::BorderColor::eIntOpaqueBlack,
                                           state.unnormalized_coordinates);
        _M_texture_sampler = API->_M_device.createSampler(sampler_info);
        return *this;
    }

    VulkanTexture& VulkanTexture::generate_mipmap()
    {
        if (!_M_image)
            return *this;

        vk::FormatProperties format_properties = API->_M_physical_device.getFormatProperties(format());

        if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        {
            throw std::runtime_error("VulkanAPI: Texture image format does not support linear blitting!");
        }

        if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst))
        {
            throw std::runtime_error("VulkanAPI: Texture image format does not support linear blitting!");
        }

        vk::CommandBuffer command_buffer = API->begin_single_time_command_buffer();

        auto subresource       = subresource_range(state.base_mip_level);
        subresource.levelCount = 1;
        subresource.layerCount = 1;

        for (uint_t layer = 0; layer < _M_layer_count; layer++)
        {
            subresource.baseArrayLayer = layer;

            vk::ImageMemoryBarrier barrier({}, {}, {}, {}, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, _M_image,
                                           subresource);
            vk::ImageMemoryBarrier barrier2 = barrier;

            for (MipMapLevel i = 1; i < state.mipmap_count; i++)
            {
                if (i >= state.base_mip_level)
                {
                    barrier.subresourceRange.baseMipLevel = i == state.base_mip_level ? 0 : i - 1;
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
                            _M_image_aspect, (i == state.base_mip_level ? 0 : i - 1), layer, 1);

                    vk::ImageSubresourceLayers dst_image_subresource_layers(_M_image_aspect, i, layer, 1);

                    auto base_mip_size    = get_mip_size(barrier.subresourceRange.baseMipLevel);
                    auto current_mip_size = get_mip_size(i);

                    const Array<vk::Offset3D, 2> src_offsets = {vk::Offset3D(0, 0, 0),
                                                                vk::Offset3D(base_mip_size.x, base_mip_size.y, 1)};

                    const Array<vk::Offset3D, 2> dst_offsets = {
                            vk::Offset3D(0, 0, 0), vk::Offset3D(current_mip_size.x, current_mip_size.y, 1)};

                    vk::ImageBlit blit(src_image_subresource_layers, src_offsets, dst_image_subresource_layers,
                                       dst_offsets);

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
        return create_image_view().create_texture_sampler();
    }

    Size2D VulkanTexture::size(MipMapLevel level)
    {
        auto out_mip_size = get_mip_size(level);
        return Size2D(static_cast<float>(out_mip_size.x), static_cast<float>(out_mip_size.y));
    }

    bool VulkanTexture::is_depth_stencil_image()
    {
        return (state.format == vk::Format::eD16Unorm || state.format == vk::Format::eD32Sfloat ||
                state.format == vk::Format::eS8Uint || state.format == vk::Format::eD32SfloatS8Uint ||
                state.format == vk::Format::eD16UnormS8Uint || state.format == vk::Format::eD24UnormS8Uint);
    }


    SamplerMipmapMode VulkanTexture::sample_mipmap_mode_texture()
    {
        return vulkan_type_to_engine_type<SamplerMipmapMode>(_M_sampler_mipmap_modes, state.sampler_mipmap_mode);
    }


    VulkanTexture& VulkanTexture::sample_mipmap_mode_texture(SamplerMipmapMode mode)
    {
        state.sampler_mipmap_mode = get_type(mode);
        return create_texture_sampler();
    }

    VulkanTexture& VulkanTexture::lod_bias_texture(LodBias bias)
    {
        state.mip_lod_bias = bias;
        return create_texture_sampler();
    }

    VulkanTexture& VulkanTexture::unnormalized_coordinates_texture(bool flag)
    {
        state.unnormalized_coordinates = flag;
        return create_texture_sampler();
    }


    template<typename Type, size_t offset = 0>
    void copy_image(const byte* vulkan_data, size_t size, Vector<byte>& out)
    {
        size_t out_size = size / (sizeof(Type) + offset);

        out.resize(out_size);

        for (size_t i = 0; i < out_size; i++)
        {
            Type value = *reinterpret_cast<const Type*>(vulkan_data);
            if (std::is_floating_point<Type>::value)
            {
                out[i] = static_cast<byte>(value * static_cast<Type>(255.0f));
            }
            else
            {
                out[i] = static_cast<byte>(value);
            }

            vulkan_data += sizeof(Type) + offset;
        }
    }


    VulkanTexture& VulkanTexture::read_texture_2D_data(Vector<byte>& data, MipMapLevel level)
    {
        if (level >= state.mipmap_count)
            return *this;

        API->wait_idle();
        auto mip_size    = get_mip_size(level);
        auto buffer_size = mip_size.x * mip_size.y * pixel_type_size();

        TransitionImageLayout transition;
        transition.image        = &_M_image;
        transition.old_layout   = vk::ImageLayout::eShaderReadOnlyOptimal;
        transition.new_layout   = vk::ImageLayout::eTransferSrcOptimal;
        transition.base_mip     = level;
        transition.mip_count    = 1;
        transition.aspect_flags = _M_image_aspect;

        transition.execute(vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead,
                           vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferRead);


        vk::BufferImageCopy copy(0, 0, 0, vk::ImageSubresourceLayers(_M_image_aspect, level, 0, 1),
                                 vk::Offset3D(0, 0, 0), vk::Extent3D(mip_size.x, mip_size.y, 1));

        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;

        API->create_buffer(buffer_size, vk::BufferUsageFlagBits::eTransferDst,
                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                           staging_buffer, staging_buffer_memory);

        auto command_buffer = API->begin_single_time_command_buffer();


        command_buffer.copyImageToBuffer(_M_image, vk::ImageLayout::eTransferSrcOptimal, staging_buffer, copy);

        API->end_single_time_command_buffer(command_buffer);

        auto mip_data =
                reinterpret_cast<const byte*>(API->_M_device.mapMemory(staging_buffer_memory, 0, VK_WHOLE_SIZE));


        switch (state.format)
        {
            case vk::Format::eR32G32B32Sfloat:
            case vk::Format::eR32G32B32A32Sfloat:
            case vk::Format::eR32Sfloat:
            case vk::Format::eD32Sfloat:
                copy_image<float, 0>(mip_data, buffer_size, data);
                break;

            case vk::Format::eR8G8B8A8Srgb:
            case vk::Format::eS8Uint:
            case vk::Format::eR8Srgb:
            case vk::Format::eR8G8B8Srgb:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eD16UnormS8Uint:
                copy_image<byte, 0>(mip_data, buffer_size, data);
                break;

            case vk::Format::eD16Unorm:
                copy_image<std::uint16_t, 0>(mip_data, buffer_size, data);
                break;

            case vk::Format::eD32SfloatS8Uint:
                copy_image<float, 1>(mip_data, buffer_size, data);
                break;
            default:
                throw std::runtime_error("Vulkan API:");
        }


        API->_M_device.unmapMemory(staging_buffer_memory);
        API->_M_device.destroyBuffer(staging_buffer);
        API->_M_device.freeMemory(staging_buffer_memory, nullptr);

        std::swap(transition.old_layout, transition.new_layout);
        transition.execute(vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferRead,
                           vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead);

        return *this;
    }

    VulkanTexture& VulkanTexture::update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel level,
                                                    uint_t layer, const void* data)
    {
        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;

        vk::DeviceSize buffer_size =
                static_cast<vk::DeviceSize>(size.x) * static_cast<vk::DeviceSize>(size.y) * pixel_type_size();


        API->create_buffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc,
                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                           staging_buffer, staging_buffer_memory);

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

        vk::BufferImageCopy region(0, 0, 0, vk::ImageSubresourceLayers(_M_image_aspect, level, layer, 1),
                                   vk::Offset3D(static_cast<uint_t>(offset.x), static_cast<uint_t>(offset.y), 0),
                                   vk::Extent3D(static_cast<uint_t>(size.x), static_cast<uint_t>(size.y), 1));

        command_buffer.copyBufferToImage(staging_buffer, _M_image, vk::ImageLayout::eTransferDstOptimal, region);

        API->end_single_time_command_buffer(command_buffer);

        API->_M_device.freeMemory(staging_buffer_memory, nullptr);
        API->_M_device.destroyBuffer(staging_buffer);

        std::swap(transition.old_layout, transition.new_layout);
        transition.execute(vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite,
                           vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead);

        return *this;
    }

    PixelType VulkanTexture::pixel_type()
    {
        switch (state.format)
        {
            case vk::Format::eR32G32B32Sfloat:
            case vk::Format::eR8G8B8Srgb:
                return PixelType::RGB;

            case vk::Format::eR32G32B32A32Sfloat:
            case vk::Format::eR8G8B8A8Srgb:
                return PixelType::RGBA;

            case vk::Format::eR8Srgb:
            case vk::Format::eR32Sfloat:
                return PixelType::Red;

            case vk::Format::eD16Unorm:
            case vk::Format::eD32Sfloat:
                return PixelType::Depth;

            case vk::Format::eS8Uint:
                return PixelType::Stencil;
            case vk::Format::eD32SfloatS8Uint:
            case vk::Format::eD16UnormS8Uint:
            case vk::Format::eD24UnormS8Uint:
                return PixelType::DepthStencil;
            default:
                throw std::runtime_error("Vulkan API: Undefiled format!");
        }
    }

    VulkanTexture& VulkanTexture::swizzle(const SwizzleRGBA& swizzle)
    {
        state.swizzle = vk::ComponentMapping(get_type(swizzle.R), get_type(swizzle.G), get_type(swizzle.B),
                                             get_type(swizzle.A));
        return create_image_view();
    }

    SwizzleRGBA VulkanTexture::swizzle()
    {
        SwizzleRGBA result;
        result.R = vulkan_type_to_engine_type<SwizzleValue>(_M_swizzle_components, state.swizzle.r);
        result.G = vulkan_type_to_engine_type<SwizzleValue>(_M_swizzle_components, state.swizzle.g);
        result.B = vulkan_type_to_engine_type<SwizzleValue>(_M_swizzle_components, state.swizzle.b);
        result.A = vulkan_type_to_engine_type<SwizzleValue>(_M_swizzle_components, state.swizzle.a);
        return result;
    }

    VulkanTexture& VulkanTexture::min_filter(TextureFilter filter)
    {
        state.min_filter = get_type(filter);
        return create_texture_sampler();
    }

    VulkanTexture& VulkanTexture::mag_filter(TextureFilter filter)
    {
        state.mag_filter = get_type(filter);
        return create_texture_sampler();
    }

    TextureFilter VulkanTexture::min_filter()
    {
        return vulkan_type_to_engine_type<TextureFilter>(_M_texture_filters, state.min_filter);
    }

    TextureFilter VulkanTexture::mag_filter()
    {
        return vulkan_type_to_engine_type<TextureFilter>(_M_texture_filters, state.mag_filter);
    }


    VulkanTexture& VulkanTexture::wrap_s(WrapValue value)
    {
        state.wrap_s = get_type(value);
        return create_texture_sampler();
    }

    VulkanTexture& VulkanTexture::wrap_t(WrapValue value)
    {
        state.wrap_t = get_type(value);
        return create_texture_sampler();
    }

    VulkanTexture& VulkanTexture::wrap_r(WrapValue value)
    {
        state.wrap_r = get_type(value);
        return create_texture_sampler();
    }

    WrapValue VulkanTexture::wrap_s()
    {
        return vulkan_type_to_engine_type<WrapValue>(_M_wrap_values, state.wrap_s);
    }

    WrapValue VulkanTexture::wrap_t()
    {
        return vulkan_type_to_engine_type<WrapValue>(_M_wrap_values, state.wrap_t);
    }

    WrapValue VulkanTexture::wrap_r()
    {
        return vulkan_type_to_engine_type<WrapValue>(_M_wrap_values, state.wrap_r);
    }

    VulkanTexture& VulkanTexture::compare_func(CompareFunc func)
    {
        state.compare_func = get_type(func);
        return create_texture_sampler();
    }

    CompareFunc VulkanTexture::compare_func()
    {
        return vulkan_type_to_engine_type<CompareFunc>(_M_compare_funcs, state.compare_func);
    }

    VulkanTexture& VulkanTexture::compare_mode(CompareMode mode)
    {
        state.compare_enable = mode == CompareMode::RefToTexture;
        return create_texture_sampler();
    }

    CompareMode VulkanTexture::compare_mode()
    {
        return state.compare_enable ? CompareMode::RefToTexture : CompareMode::None;
    }


    VulkanTexture& VulkanTexture::base_level(MipMapLevel level)
    {
        state.base_mip_level = glm::min(level, state.mipmap_count);
        return create_image_view();
    }

    VulkanTexture& VulkanTexture::min_lod_level(LodLevel level)
    {
        state.min_lod = level;
        return create_texture_sampler();
    }

    VulkanTexture& VulkanTexture::max_lod_level(LodLevel level)
    {
        state.max_lod = level;
        return create_texture_sampler();
    }

    VulkanTexture& VulkanTexture::clear()
    {
        API->wait_idle();
        DESTROY_CALL(destroyImage, _M_image);
        DESTROY_CALL(freeMemory, _M_image_memory);
        DESTROY_CALL(destroyImageView, _M_image_view);
        DESTROY_CALL(destroySampler, _M_texture_sampler);
        return *this;
    }

    vk::Offset2D VulkanTexture::get_mip_size(MipMapLevel level)
    {
        uint_t width  = glm::max(static_cast<uint_t>(state.size.width) >> level, 1U);
        uint_t height = glm::max(static_cast<uint_t>(state.size.height) >> level, 1U);
        return vk::Offset2D(width, height);
    }

    VulkanTexture& VulkanTexture::anisotropic_value(float value)
    {
        state.anisotropy        = glm::min(glm::max(value, 1.0f), API->max_anisotropic_filtering());
        state.anisotropy_enable = state.anisotropy > 1.0;
        return create_texture_sampler();
    }

    bool VulkanTexture::can_use_color_as_color_attachment()
    {
        for (auto& format : color_formats[static_cast<size_t>(PixelType::RGBA)])
        {
            if (state.format == format)
                return true;
        }

        return false;
    }

    VulkanTexture::~VulkanTexture()
    {
        clear();
    }

}// namespace Engine
