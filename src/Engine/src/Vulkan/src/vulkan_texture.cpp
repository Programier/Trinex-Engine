#include <vulkan_api.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{


    void* VulkanTexture::get_instance_data()
    {
        return this;
    }

    VulkanTexture::VulkanTexture()
    {}


    VulkanTexture& VulkanTexture::init(const TextureParams& params)
    {
        _M_params = params;
        return *this;
    }

    size_t VulkanTexture::pixel_component_count()
    {
        switch (_M_params.pixel_type)
        {
            case Engine::PixelType::Red:
                return 1;
            case Engine::PixelType::RGB:
                return 3;
            case Engine::PixelType::RGBA:
                return 4;
            default:
                return 1;
        }
    }

    vk::ImageViewType VulkanTexture::image_view_type()
    {
#define RETURN_TYPE(type)                                                                                              \
    case vk::ImageType::type:                                                                                          \
        return vk::ImageViewType::type

        switch (_M_type)
        {
            RETURN_TYPE(e1D);
            RETURN_TYPE(e2D);
            RETURN_TYPE(e3D);
            default:
                throw std::runtime_error("VulkanAPI: Texture type does't support!");
        }
#undef RETURN_TYPE
    }


#define SIZE(component) static_cast<size_t>(_M_size.component)
    VulkanTexture& VulkanTexture::create_image()
    {
        vk::ImageCreateInfo image_info({}, _M_type, format(), vk::Extent3D(SIZE(x), SIZE(y), SIZE(z)), _M_mip_levels, 1,
                                       vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
                                       vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                       vk::SharingMode::eExclusive);

        _M_image = API->_M_device.createImage(image_info);
        vk::MemoryRequirements memory_requirements = API->_M_device.getImageMemoryRequirements(_M_image);
        vk::MemoryAllocateInfo alloc_info(
                memory_requirements.size,
                API->find_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostCoherent));
        _M_image_memory = API->_M_device.allocateMemory(alloc_info);
        API->_M_device.bindImageMemory(_M_image, _M_image_memory, 0);
        return *this;
    }

    vk::Format VulkanTexture::format()
    {
        if (_M_params.pixel_component_type == Engine::PixelComponentType::UnsignedByte)
        {
            switch (_M_params.pixel_type)
            {
                case Engine::PixelType::RGB:
                    return vk::Format::eR8G8B8Srgb;
                case Engine::PixelType::RGBA:
                    return vk::Format::eR8G8B8A8Srgb;
                case Engine::PixelType::Red:
                    return vk::Format::eR8Srgb;

                default:
                    throw std::runtime_error("VulkanAPI: Pixel type does't support!");
            }
        }
        else if (_M_params.pixel_component_type == Engine::PixelComponentType::Float)
        {
            switch (_M_params.pixel_type)
            {
                case Engine::PixelType::RGB:
                    return vk::Format::eR32G32B32Sfloat;
                case Engine::PixelType::RGBA:
                    return vk::Format::eR32G32B32A32Sfloat;
                case Engine::PixelType::Red:
                    return vk::Format::eR32Sfloat;
                default:
                    throw std::runtime_error("VulkanAPI: Pixel type does't support!");
            }
        }

        throw std::runtime_error("VulkanAPI: Undefined image format");
    }

    vk::ImageSubresourceRange VulkanTexture::subresource_range()
    {
        return vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, _M_mip_levels, 0, 1);
    }

    VulkanTexture& VulkanTexture::transition_image_layout(vk::ImageLayout old_layout, vk::ImageLayout new_layout)
    {
        vk::CommandBuffer command_buffer = API->begin_single_time_command_buffer();


        vk::ImageMemoryBarrier barrier({}, {}, old_layout, new_layout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                                       _M_image, subresource_range());

        vk::PipelineStageFlags source_stage;
        vk::PipelineStageFlags destination_stage;

        if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eNone;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
            destination_stage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (old_layout == vk::ImageLayout::eTransferDstOptimal &&
                 new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            source_stage = vk::PipelineStageFlagBits::eTransfer;
            destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else
        {
            throw std::invalid_argument("VulkanAPI: Unsupported layout transition!");
        }

        command_buffer.pipelineBarrier(source_stage, destination_stage, {}, {}, {}, barrier);
        API->end_single_time_command_buffer(command_buffer);
        return *this;
    }

    VulkanTexture& VulkanTexture::copy_buffer_to_image(vk::Buffer buffer)
    {
        vk::CommandBuffer command_buffer = API->begin_single_time_command_buffer();

        vk::BufferImageCopy region(0, 0, 0, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                                   vk::Offset3D(0, 0, 0), vk::Extent3D(SIZE(x), SIZE(y), SIZE(z)));

        command_buffer.copyBufferToImage(buffer, _M_image, vk::ImageLayout::eTransferDstOptimal, region);

        API->end_single_time_command_buffer(command_buffer);
        return *this;
    }

    VulkanTexture& VulkanTexture::gen_texture_2D(const Size2D& size, int_t mipmap, void* data)
    {
        _M_type = vk::ImageType::e2D;
        _M_size = Size3D(size, 1.0);
        vk::DeviceSize image_size =
                static_cast<vk::DeviceSize>(size.x) * static_cast<vk::DeviceSize>(size.y) * pixel_component_count();

        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;
        API->create_buffer(image_size, vk::BufferUsageFlagBits::eTransferSrc,
                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                           staging_buffer, staging_buffer_memory);


        void* vulkan_data = API->_M_device.mapMemory(staging_buffer_memory, 0, image_size);
        std::memcpy(vulkan_data, data, static_cast<size_t>(image_size));
        API->_M_device.unmapMemory(staging_buffer_memory);
        create_image()
                .transition_image_layout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal)
                .copy_buffer_to_image(staging_buffer)
                .transition_image_layout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        API->_M_device.destroyBuffer(staging_buffer);
        API->_M_device.freeMemory(staging_buffer_memory, nullptr);

        return create_image_view().create_texture_sampler();
    }

    VulkanTexture& VulkanTexture::create_image_view()
    {
        vk::ImageViewCreateInfo view_info({}, _M_image, image_view_type(), format(), {}, subresource_range());
        _M_image_view = API->_M_device.createImageView(view_info);
        return *this;
    }

    VulkanTexture& VulkanTexture::create_texture_sampler()
    {
        DESTROY_CALL(destroySampler, _M_texture_sampler);
        vk::PhysicalDeviceProperties properties = API->_M_physical_device.getProperties();

        vk::SamplerCreateInfo sampler_info({}, vk::Filter::eNearest, vk::Filter::eNearest,
                                           vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat,
                                           vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, {},
                                           VK_TRUE, properties.limits.maxSamplerAnisotropy, VK_FALSE,
                                           vk::CompareOp::eAlways, {}, {}, vk::BorderColor::eIntOpaqueBlack, VK_FALSE);

        _M_texture_sampler = API->_M_device.createSampler(sampler_info);
        return *this;
    }

    VulkanTexture::~VulkanTexture()
    {
        DESTROY_CALL(destroyImage, _M_image);
        DESTROY_CALL(freeMemory, _M_image_memory);
        DESTROY_CALL(destroyImageView, _M_image_view);
        DESTROY_CALL(destroySampler, _M_texture_sampler);
    }
}// namespace Engine
