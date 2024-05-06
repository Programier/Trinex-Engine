#include <vulkan_api.hpp>

namespace Engine
{
    vk::Format parse_engine_format(ColorFormat format)
    {
        switch (format)
        {
            case ColorFormat::Undefined:
                return vk::Format::eUndefined;

            case ColorFormat::FloatR:
                return vk::Format::eR32Sfloat;
            case ColorFormat::FloatRG:
                return vk::Format::eR32G32Sfloat;
            case ColorFormat::FloatRGB:
                return vk::Format::eR32G32B32Sfloat;
            case ColorFormat::FloatRGBA:
                return vk::Format::eR32G32B32A32Sfloat;
            case ColorFormat::R8:
                return vk::Format::eR8Unorm;
            case ColorFormat::R8G8:
                return vk::Format::eR8G8Unorm;
            case ColorFormat::R8G8B8:
                return vk::Format::eR8G8B8Unorm;
            case ColorFormat::R8G8B8A8:
                return vk::Format::eR8G8B8A8Unorm;
            case ColorFormat::DepthStencil:
                return vk::Format::eD32SfloatS8Uint;
            case ColorFormat::ShadowDepth:
                return vk::Format::eD32Sfloat;
            case ColorFormat::FilteredShadowDepth:
                return vk::Format::eD32Sfloat;
            case ColorFormat::D32F:
                return vk::Format::eD32Sfloat;
            case ColorFormat::BC1:
                return vk::Format::eBc1RgbaUnormBlock;
            case ColorFormat::BC2:
                return vk::Format::eBc2UnormBlock;
            case ColorFormat::BC3:
                return vk::Format::eBc3UnormBlock;

            default:
                return vk::Format::eUndefined;
        }
    }
}// namespace Engine
