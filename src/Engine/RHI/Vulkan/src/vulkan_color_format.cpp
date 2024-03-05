#include <vulkan_api.hpp>

namespace Engine
{
    vk::Format parse_engine_format(ColorFormat format)
    {
        switch (format)
        {
            case ColorFormat::Undefined:
                return vk::Format::eUndefined;
            case ColorFormat::R8Unorm:
                return vk::Format::eR8Unorm;
            case ColorFormat::R8Snorm:
                return vk::Format::eR8Snorm;
            case ColorFormat::R8Uscaled:
                return vk::Format::eR8Uscaled;
            case ColorFormat::R8Sscaled:
                return vk::Format::eR8Sscaled;
            case ColorFormat::R8Uint:
                return vk::Format::eR8Uint;
            case ColorFormat::R8Sint:
                return vk::Format::eR8Sint;
            case ColorFormat::R8Srgb:
                return vk::Format::eR8Srgb;
            case ColorFormat::R8G8Unorm:
                return vk::Format::eR8G8Unorm;
            case ColorFormat::R8G8Snorm:
                return vk::Format::eR8G8Snorm;
            case ColorFormat::R8G8Uscaled:
                return vk::Format::eR8G8Uscaled;
            case ColorFormat::R8G8Sscaled:
                return vk::Format::eR8G8Sscaled;
            case ColorFormat::R8G8Uint:
                return vk::Format::eR8G8Uint;
            case ColorFormat::R8G8Sint:
                return vk::Format::eR8G8Sint;
            case ColorFormat::R8G8Srgb:
                return vk::Format::eR8G8Srgb;
            case ColorFormat::R8G8B8Unorm:
                return vk::Format::eR8G8B8Unorm;
            case ColorFormat::R8G8B8Snorm:
                return vk::Format::eR8G8B8Snorm;
            case ColorFormat::R8G8B8Uscaled:
                return vk::Format::eR8G8B8Uscaled;
            case ColorFormat::R8G8B8Sscaled:
                return vk::Format::eR8G8B8Sscaled;
            case ColorFormat::R8G8B8Uint:
                return vk::Format::eR8G8B8Uint;
            case ColorFormat::R8G8B8Sint:
                return vk::Format::eR8G8B8Sint;
            case ColorFormat::R8G8B8Srgb:
                return vk::Format::eR8G8B8Srgb;
            case ColorFormat::B8G8R8Unorm:
                return vk::Format::eB8G8R8Unorm;
            case ColorFormat::B8G8R8Snorm:
                return vk::Format::eB8G8R8Snorm;
            case ColorFormat::B8G8R8Uscaled:
                return vk::Format::eB8G8R8Uscaled;
            case ColorFormat::B8G8R8Sscaled:
                return vk::Format::eB8G8R8Sscaled;
            case ColorFormat::B8G8R8Uint:
                return vk::Format::eB8G8R8Uint;
            case ColorFormat::B8G8R8Sint:
                return vk::Format::eB8G8R8Sint;
            case ColorFormat::B8G8R8Srgb:
                return vk::Format::eB8G8R8Srgb;
            case ColorFormat::R8G8B8A8Unorm:
                return vk::Format::eR8G8B8A8Unorm;
            case ColorFormat::R8G8B8A8Snorm:
                return vk::Format::eR8G8B8A8Snorm;
            case ColorFormat::R8G8B8A8Uscaled:
                return vk::Format::eR8G8B8A8Uscaled;
            case ColorFormat::R8G8B8A8Sscaled:
                return vk::Format::eR8G8B8A8Sscaled;
            case ColorFormat::R8G8B8A8Uint:
                return vk::Format::eR8G8B8A8Uint;
            case ColorFormat::R8G8B8A8Sint:
                return vk::Format::eR8G8B8A8Sint;
            case ColorFormat::R8G8B8A8Srgb:
                return vk::Format::eR8G8B8A8Srgb;
            case ColorFormat::B8G8R8A8Unorm:
                return vk::Format::eB8G8R8A8Unorm;
            case ColorFormat::B8G8R8A8Snorm:
                return vk::Format::eB8G8R8A8Snorm;
            case ColorFormat::B8G8R8A8Uscaled:
                return vk::Format::eB8G8R8A8Uscaled;
            case ColorFormat::B8G8R8A8Sscaled:
                return vk::Format::eB8G8R8A8Sscaled;
            case ColorFormat::B8G8R8A8Uint:
                return vk::Format::eB8G8R8A8Uint;
            case ColorFormat::B8G8R8A8Sint:
                return vk::Format::eB8G8R8A8Sint;
            case ColorFormat::B8G8R8A8Srgb:
                return vk::Format::eB8G8R8A8Srgb;
            case ColorFormat::R16Unorm:
                return vk::Format::eR16Unorm;
            case ColorFormat::R16Snorm:
                return vk::Format::eR16Snorm;
            case ColorFormat::R16Uscaled:
                return vk::Format::eR16Uscaled;
            case ColorFormat::R16Sscaled:
                return vk::Format::eR16Sscaled;
            case ColorFormat::R16Uint:
                return vk::Format::eR16Uint;
            case ColorFormat::R16Sint:
                return vk::Format::eR16Sint;
            case ColorFormat::R16Sfloat:
                return vk::Format::eR16Sfloat;
            case ColorFormat::R16G16Unorm:
                return vk::Format::eR16G16Unorm;
            case ColorFormat::R16G16Snorm:
                return vk::Format::eR16G16Snorm;
            case ColorFormat::R16G16Uscaled:
                return vk::Format::eR16G16Uscaled;
            case ColorFormat::R16G16Sscaled:
                return vk::Format::eR16G16Sscaled;
            case ColorFormat::R16G16Uint:
                return vk::Format::eR16G16Uint;
            case ColorFormat::R16G16Sint:
                return vk::Format::eR16G16Sint;
            case ColorFormat::R16G16Sfloat:
                return vk::Format::eR16G16Sfloat;
            case ColorFormat::R16G16B16Unorm:
                return vk::Format::eR16G16B16Unorm;
            case ColorFormat::R16G16B16Snorm:
                return vk::Format::eR16G16B16Snorm;
            case ColorFormat::R16G16B16Uscaled:
                return vk::Format::eR16G16B16Uscaled;
            case ColorFormat::R16G16B16Sscaled:
                return vk::Format::eR16G16B16Sscaled;
            case ColorFormat::R16G16B16Uint:
                return vk::Format::eR16G16B16Uint;
            case ColorFormat::R16G16B16Sint:
                return vk::Format::eR16G16B16Sint;
            case ColorFormat::R16G16B16Sfloat:
                return vk::Format::eR16G16B16Sfloat;
            case ColorFormat::R16G16B16A16Unorm:
                return vk::Format::eR16G16B16A16Unorm;
            case ColorFormat::R16G16B16A16Snorm:
                return vk::Format::eR16G16B16A16Snorm;
            case ColorFormat::R16G16B16A16Uscaled:
                return vk::Format::eR16G16B16A16Uscaled;
            case ColorFormat::R16G16B16A16Sscaled:
                return vk::Format::eR16G16B16A16Sscaled;
            case ColorFormat::R16G16B16A16Uint:
                return vk::Format::eR16G16B16A16Uint;
            case ColorFormat::R16G16B16A16Sint:
                return vk::Format::eR16G16B16A16Sint;
            case ColorFormat::R16G16B16A16Sfloat:
                return vk::Format::eR16G16B16A16Sfloat;
            case ColorFormat::R32Uint:
                return vk::Format::eR32Uint;
            case ColorFormat::R32Sint:
                return vk::Format::eR32Sint;
            case ColorFormat::R32Sfloat:
                return vk::Format::eR32Sfloat;
            case ColorFormat::R32G32Uint:
                return vk::Format::eR32G32Uint;
            case ColorFormat::R32G32Sint:
                return vk::Format::eR32G32Sint;
            case ColorFormat::R32G32Sfloat:
                return vk::Format::eR32G32Sfloat;
            case ColorFormat::R32G32B32Uint:
                return vk::Format::eR32G32B32Uint;
            case ColorFormat::R32G32B32Sint:
                return vk::Format::eR32G32B32Sint;
            case ColorFormat::R32G32B32Sfloat:
                return vk::Format::eR32G32B32Sfloat;
            case ColorFormat::R32G32B32A32Uint:
                return vk::Format::eR32G32B32A32Uint;
            case ColorFormat::R32G32B32A32Sint:
                return vk::Format::eR32G32B32A32Sint;
            case ColorFormat::R32G32B32A32Sfloat:
                return vk::Format::eR32G32B32A32Sfloat;
            case ColorFormat::D16Unorm:
                return vk::Format::eD16Unorm;
            case ColorFormat::D32Sfloat:
                return vk::Format::eD32Sfloat;
            case ColorFormat::S8Uint:
                return vk::Format::eD16UnormS8Uint;
            case ColorFormat::D16UnormS8Uint:
                return vk::Format::eD16UnormS8Uint;
            case ColorFormat::D24UnormS8Uint:
                return vk::Format::eD24UnormS8Uint;
            case ColorFormat::D32SfloatS8Uint:
                return vk::Format::eD32SfloatS8Uint;
            case ColorFormat::BC1Unorm:
                return vk::Format::eBc1RgbUnormBlock;
            case ColorFormat::BC3Unorm:
                return vk::Format::eBc3UnormBlock;

            default:
                return vk::Format::eUndefined;
        }
    }

    ColorFormat to_engine_format(vk::Format format)
    {
        switch (format)
        {
            case vk::Format::eUndefined:
                return ColorFormat::Undefined;
            case vk::Format::eR8Unorm:
                return ColorFormat::R8Unorm;
            case vk::Format::eR8Snorm:
                return ColorFormat::R8Snorm;
            case vk::Format::eR8Uscaled:
                return ColorFormat::R8Uscaled;
            case vk::Format::eR8Sscaled:
                return ColorFormat::R8Sscaled;
            case vk::Format::eR8Uint:
                return ColorFormat::R8Uint;
            case vk::Format::eR8Sint:
                return ColorFormat::R8Sint;
            case vk::Format::eR8Srgb:
                return ColorFormat::R8Srgb;
            case vk::Format::eR8G8Unorm:
                return ColorFormat::R8G8Unorm;
            case vk::Format::eR8G8Snorm:
                return ColorFormat::R8G8Snorm;
            case vk::Format::eR8G8Uscaled:
                return ColorFormat::R8G8Uscaled;
            case vk::Format::eR8G8Sscaled:
                return ColorFormat::R8G8Sscaled;
            case vk::Format::eR8G8Uint:
                return ColorFormat::R8G8Uint;
            case vk::Format::eR8G8Sint:
                return ColorFormat::R8G8Sint;
            case vk::Format::eR8G8Srgb:
                return ColorFormat::R8G8Srgb;
            case vk::Format::eR8G8B8Unorm:
                return ColorFormat::R8G8B8Unorm;
            case vk::Format::eR8G8B8Snorm:
                return ColorFormat::R8G8B8Snorm;
            case vk::Format::eR8G8B8Uscaled:
                return ColorFormat::R8G8B8Uscaled;
            case vk::Format::eR8G8B8Sscaled:
                return ColorFormat::R8G8B8Sscaled;
            case vk::Format::eR8G8B8Uint:
                return ColorFormat::R8G8B8Uint;
            case vk::Format::eR8G8B8Sint:
                return ColorFormat::R8G8B8Sint;
            case vk::Format::eR8G8B8Srgb:
                return ColorFormat::R8G8B8Srgb;
            case vk::Format::eB8G8R8Unorm:
                return ColorFormat::B8G8R8Unorm;
            case vk::Format::eB8G8R8Snorm:
                return ColorFormat::B8G8R8Snorm;
            case vk::Format::eB8G8R8Uscaled:
                return ColorFormat::B8G8R8Uscaled;
            case vk::Format::eB8G8R8Sscaled:
                return ColorFormat::B8G8R8Sscaled;
            case vk::Format::eB8G8R8Uint:
                return ColorFormat::B8G8R8Uint;
            case vk::Format::eB8G8R8Sint:
                return ColorFormat::B8G8R8Sint;
            case vk::Format::eB8G8R8Srgb:
                return ColorFormat::B8G8R8Srgb;
            case vk::Format::eR8G8B8A8Unorm:
                return ColorFormat::R8G8B8A8Unorm;
            case vk::Format::eR8G8B8A8Snorm:
                return ColorFormat::R8G8B8A8Snorm;
            case vk::Format::eR8G8B8A8Uscaled:
                return ColorFormat::R8G8B8A8Uscaled;
            case vk::Format::eR8G8B8A8Sscaled:
                return ColorFormat::R8G8B8A8Sscaled;
            case vk::Format::eR8G8B8A8Uint:
                return ColorFormat::R8G8B8A8Uint;
            case vk::Format::eR8G8B8A8Sint:
                return ColorFormat::R8G8B8A8Sint;
            case vk::Format::eR8G8B8A8Srgb:
                return ColorFormat::R8G8B8A8Srgb;
            case vk::Format::eB8G8R8A8Unorm:
                return ColorFormat::B8G8R8A8Unorm;
            case vk::Format::eB8G8R8A8Snorm:
                return ColorFormat::B8G8R8A8Snorm;
            case vk::Format::eB8G8R8A8Uscaled:
                return ColorFormat::B8G8R8A8Uscaled;
            case vk::Format::eB8G8R8A8Sscaled:
                return ColorFormat::B8G8R8A8Sscaled;
            case vk::Format::eB8G8R8A8Uint:
                return ColorFormat::B8G8R8A8Uint;
            case vk::Format::eB8G8R8A8Sint:
                return ColorFormat::B8G8R8A8Sint;
            case vk::Format::eB8G8R8A8Srgb:
                return ColorFormat::B8G8R8A8Srgb;
            case vk::Format::eR16Unorm:
                return ColorFormat::R16Unorm;
            case vk::Format::eR16Snorm:
                return ColorFormat::R16Snorm;
            case vk::Format::eR16Uscaled:
                return ColorFormat::R16Uscaled;
            case vk::Format::eR16Sscaled:
                return ColorFormat::R16Sscaled;
            case vk::Format::eR16Uint:
                return ColorFormat::R16Uint;
            case vk::Format::eR16Sint:
                return ColorFormat::R16Sint;
            case vk::Format::eR16Sfloat:
                return ColorFormat::R16Sfloat;
            case vk::Format::eR16G16Unorm:
                return ColorFormat::R16G16Unorm;
            case vk::Format::eR16G16Snorm:
                return ColorFormat::R16G16Snorm;
            case vk::Format::eR16G16Uscaled:
                return ColorFormat::R16G16Uscaled;
            case vk::Format::eR16G16Sscaled:
                return ColorFormat::R16G16Sscaled;
            case vk::Format::eR16G16Uint:
                return ColorFormat::R16G16Uint;
            case vk::Format::eR16G16Sint:
                return ColorFormat::R16G16Sint;
            case vk::Format::eR16G16Sfloat:
                return ColorFormat::R16G16Sfloat;
            case vk::Format::eR16G16B16Unorm:
                return ColorFormat::R16G16B16Unorm;
            case vk::Format::eR16G16B16Snorm:
                return ColorFormat::R16G16B16Snorm;
            case vk::Format::eR16G16B16Uscaled:
                return ColorFormat::R16G16B16Uscaled;
            case vk::Format::eR16G16B16Sscaled:
                return ColorFormat::R16G16B16Sscaled;
            case vk::Format::eR16G16B16Uint:
                return ColorFormat::R16G16B16Uint;
            case vk::Format::eR16G16B16Sint:
                return ColorFormat::R16G16B16Sint;
            case vk::Format::eR16G16B16Sfloat:
                return ColorFormat::R16G16B16Sfloat;
            case vk::Format::eR16G16B16A16Unorm:
                return ColorFormat::R16G16B16A16Unorm;
            case vk::Format::eR16G16B16A16Snorm:
                return ColorFormat::R16G16B16A16Snorm;
            case vk::Format::eR16G16B16A16Uscaled:
                return ColorFormat::R16G16B16A16Uscaled;
            case vk::Format::eR16G16B16A16Sscaled:
                return ColorFormat::R16G16B16A16Sscaled;
            case vk::Format::eR16G16B16A16Uint:
                return ColorFormat::R16G16B16A16Uint;
            case vk::Format::eR16G16B16A16Sint:
                return ColorFormat::R16G16B16A16Sint;
            case vk::Format::eR16G16B16A16Sfloat:
                return ColorFormat::R16G16B16A16Sfloat;
            case vk::Format::eR32Uint:
                return ColorFormat::R32Uint;
            case vk::Format::eR32Sint:
                return ColorFormat::R32Sint;
            case vk::Format::eR32Sfloat:
                return ColorFormat::R32Sfloat;
            case vk::Format::eR32G32Uint:
                return ColorFormat::R32G32Uint;
            case vk::Format::eR32G32Sint:
                return ColorFormat::R32G32Sint;
            case vk::Format::eR32G32Sfloat:
                return ColorFormat::R32G32Sfloat;
            case vk::Format::eR32G32B32Uint:
                return ColorFormat::R32G32B32Uint;
            case vk::Format::eR32G32B32Sint:
                return ColorFormat::R32G32B32Sint;
            case vk::Format::eR32G32B32Sfloat:
                return ColorFormat::R32G32B32Sfloat;
            case vk::Format::eR32G32B32A32Uint:
                return ColorFormat::R32G32B32A32Uint;
            case vk::Format::eR32G32B32A32Sint:
                return ColorFormat::R32G32B32A32Sint;
            case vk::Format::eR32G32B32A32Sfloat:
                return ColorFormat::R32G32B32A32Sfloat;
            case vk::Format::eD16Unorm:
                return ColorFormat::D16Unorm;
            case vk::Format::eD32Sfloat:
                return ColorFormat::D32Sfloat;
            case vk::Format::eS8Uint:
                return ColorFormat::S8Uint;
            case vk::Format::eD16UnormS8Uint:
                return ColorFormat::D16UnormS8Uint;
            case vk::Format::eD24UnormS8Uint:
                return ColorFormat::D24UnormS8Uint;
            case vk::Format::eD32SfloatS8Uint:
                return ColorFormat::D32SfloatS8Uint;

            default:
                return ColorFormat::Undefined;
        }
    }


    static struct VulkanColorFormats {
        ColorFormat base_color           = ColorFormat::Undefined;
        ColorFormat position_format      = ColorFormat::Undefined;
        ColorFormat normal_format        = ColorFormat::Undefined;
        ColorFormat emissive_format      = ColorFormat::Undefined;
        ColorFormat msra_format          = ColorFormat::Undefined;
        ColorFormat depth_format         = ColorFormat::Undefined;
        ColorFormat stencil_format       = ColorFormat::Undefined;
        ColorFormat depth_stencil_format = ColorFormat::Undefined;

        static ColorFormat find_format(ColorFormat* formats, size_t size, vk::FormatFeatureFlagBits flags)
        {
            for (size_t i = 0; i < size; ++i)
            {
                auto v_format = parse_engine_format(formats[i]);
                auto props    = API->m_physical_device.getFormatProperties(v_format);
                if ((props.optimalTilingFeatures & flags))
                {
                    return formats[i];
                }
            }

            return ColorFormat::Undefined;
        }

        void init()
        {
            {
                ColorFormat formats[] = {ColorFormat::R8G8B8A8Unorm, ColorFormat::R32G32B32A32Sfloat,
                                         ColorFormat::R16G16B16A16Sfloat};

                base_color = find_format(formats, sizeof(formats) / sizeof(ColorFormat),
                                         vk::FormatFeatureFlagBits::eColorAttachmentBlend);

                if (base_color == ColorFormat::Undefined)
                {
                    find_format(formats, sizeof(formats) / sizeof(ColorFormat), vk::FormatFeatureFlagBits::eColorAttachment);
                }
            }

            {
                ColorFormat formats[] = {ColorFormat::R16G16B16A16Sfloat, ColorFormat::R32G32B32A32Sfloat};
                position_format =
                        find_format(formats, sizeof(formats) / sizeof(ColorFormat), vk::FormatFeatureFlagBits::eColorAttachment);
            }

            normal_format   = position_format;
            emissive_format = base_color;
            msra_format     = base_color;

            {
                ColorFormat formats[] = {ColorFormat::D16Unorm, ColorFormat::D32Sfloat};
                depth_format          = find_format(formats, sizeof(formats) / sizeof(ColorFormat),
                                                    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
            }

            {
                ColorFormat formats[] = {ColorFormat::S8Uint};
                stencil_format        = find_format(formats, sizeof(formats) / sizeof(ColorFormat),
                                                    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
            }

            {
                ColorFormat formats[] = {ColorFormat::D16UnormS8Uint, ColorFormat::D24UnormS8Uint, ColorFormat::D32SfloatS8Uint};
                depth_stencil_format  = find_format(formats, sizeof(formats) / sizeof(ColorFormat),
                                                    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
            }
        }
    } formats;

    void VulkanAPI::initialize_color_formats()
    {
        formats.init();
    }

    ColorFormat VulkanAPI::base_color_format()
    {
        return formats.base_color;
    }

    ColorFormat VulkanAPI::position_format()
    {
        return formats.position_format;
    }

    ColorFormat VulkanAPI::normal_format()
    {
        return formats.normal_format;
    }

    ColorFormat VulkanAPI::emissive_format()
    {
        return formats.emissive_format;
    }

    ColorFormat VulkanAPI::msra_buffer_format()
    {
        return formats.msra_format;
    }

    ColorFormat VulkanAPI::depth_format()
    {
        return formats.depth_format;
    }

    ColorFormat VulkanAPI::stencil_format()
    {
        return formats.stencil_format;
    }

    ColorFormat VulkanAPI::depth_stencil_format()
    {
        return formats.depth_stencil_format;
    }
}// namespace Engine
