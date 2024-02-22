#include <Core/color_format.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/enum.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{

    ColorFormatFeatures::ColorFormatFeatures() : data(0)
    {}

    bool ColorFormatFeatures::contains(const ColorFormatFeatures& other) const
    {
        static constexpr size_t array_size = sizeof(data);
        for (size_t i = 0; i < array_size; i++)
        {
            if ((data[i] & other.data[i]) != other.data[i])
                return false;
        }

        return true;
    }

    static Map<ColorFormat, ColorFormatFeatures>& static_color_format_features()
    {
        static Map<ColorFormat, ColorFormatFeatures> features;
        return features;
    }

    const ColorFormatFeatures& ColorFormatInfo::features() const
    {
        static ColorFormatFeatures default_features;
        auto it = static_color_format_features().find(static_cast<ColorFormat>(m_value));

        if (it != static_color_format_features().end())
        {
            return it->second;
        }

        return default_features;
    }

    const Vector<ColorFormat>& ColorFormatInfo::all_formats()
    {
        static const Vector<ColorFormat> formats = {
                ColorFormat::Undefined,
                ColorFormat::R8Unorm,
                ColorFormat::R8Snorm,
                ColorFormat::R8Uscaled,
                ColorFormat::R8Sscaled,
                ColorFormat::S8Uint,
                ColorFormat::R8Uint,
                ColorFormat::R8Sint,
                ColorFormat::R8Srgb,
                ColorFormat::R16Unorm,
                ColorFormat::D16Unorm,
                ColorFormat::R16Snorm,
                ColorFormat::R16Uscaled,
                ColorFormat::R16Sscaled,
                ColorFormat::R16Uint,
                ColorFormat::R16Sint,
                ColorFormat::R16Sfloat,
                ColorFormat::D32Sfloat,
                ColorFormat::R32Sfloat,
                ColorFormat::R32Uint,
                ColorFormat::R32Sint,
                ColorFormat::R8G8Unorm,
                ColorFormat::R8G8Snorm,
                ColorFormat::R8G8Uscaled,
                ColorFormat::R8G8Sscaled,
                ColorFormat::R8G8Uint,
                ColorFormat::R8G8Sint,
                ColorFormat::R8G8Srgb,
                ColorFormat::R16G16Unorm,
                ColorFormat::R16G16Snorm,
                ColorFormat::R16G16Uscaled,
                ColorFormat::R16G16Sscaled,
                ColorFormat::R16G16Uint,
                ColorFormat::R16G16Sint,
                ColorFormat::R16G16Sfloat,
                ColorFormat::D16UnormS8Uint,
                ColorFormat::D24UnormS8Uint,
                ColorFormat::R32G32Uint,
                ColorFormat::R32G32Sint,
                ColorFormat::R32G32Sfloat,
                ColorFormat::D32SfloatS8Uint,
                ColorFormat::R8G8B8Unorm,
                ColorFormat::B8G8R8Unorm,
                ColorFormat::R8G8B8Snorm,
                ColorFormat::B8G8R8Snorm,
                ColorFormat::R8G8B8Uscaled,
                ColorFormat::B8G8R8Uscaled,
                ColorFormat::R8G8B8Sscaled,
                ColorFormat::B8G8R8Sscaled,
                ColorFormat::R8G8B8Uint,
                ColorFormat::B8G8R8Uint,
                ColorFormat::R8G8B8Sint,
                ColorFormat::B8G8R8Sint,
                ColorFormat::R8G8B8Srgb,
                ColorFormat::B8G8R8Srgb,
                ColorFormat::R16G16B16Unorm,
                ColorFormat::R16G16B16Snorm,
                ColorFormat::R16G16B16Uscaled,
                ColorFormat::R16G16B16Sscaled,
                ColorFormat::R16G16B16Uint,
                ColorFormat::R16G16B16Sint,
                ColorFormat::R16G16B16Sfloat,
                ColorFormat::R32G32B32Uint,
                ColorFormat::R32G32B32Sint,
                ColorFormat::R32G32B32Sfloat,
                ColorFormat::R8G8B8A8Unorm,
                ColorFormat::B8G8R8A8Unorm,
                ColorFormat::R8G8B8A8Snorm,
                ColorFormat::B8G8R8A8Snorm,
                ColorFormat::R8G8B8A8Uscaled,
                ColorFormat::B8G8R8A8Uscaled,
                ColorFormat::R8G8B8A8Sscaled,
                ColorFormat::B8G8R8A8Sscaled,
                ColorFormat::R8G8B8A8Uint,
                ColorFormat::B8G8R8A8Uint,
                ColorFormat::R8G8B8A8Sint,
                ColorFormat::B8G8R8A8Sint,
                ColorFormat::R8G8B8A8Srgb,
                ColorFormat::B8G8R8A8Srgb,
                ColorFormat::R16G16B16A16Unorm,
                ColorFormat::R16G16B16A16Snorm,
                ColorFormat::R16G16B16A16Uscaled,
                ColorFormat::R16G16B16A16Sscaled,
                ColorFormat::R16G16B16A16Uint,
                ColorFormat::R16G16B16A16Sint,
                ColorFormat::R16G16B16A16Sfloat,
                ColorFormat::R32G32B32A32Uint,
                ColorFormat::R32G32B32A32Sint,
                ColorFormat::R32G32B32A32Sfloat,
        };

        return formats;
    }

    static void on_rhi_init()
    {
        auto& set                                = static_color_format_features();
        const Vector<ColorFormat>& color_formats = ColorFormatInfo::all_formats();
        RHI* rhi                                 = engine_instance->rhi();
        for (auto& format : color_formats)
        {
            set[format] = rhi->color_format_features(format);
        }
    }

    static AfterRHIInitializeController controller(on_rhi_init);

    implement_enum(ColorFormat, Engine, {"Undefined", ColorFormat::Undefined}, {"R8Unorm", ColorFormat::R8Unorm},
                   {"R8Snorm", ColorFormat::R8Snorm}, {"R8Uscaled", ColorFormat::R8Uscaled},
                   {"R8Sscaled", ColorFormat::R8Sscaled}, {"S8Uint", ColorFormat::S8Uint}, {"R8Uint", ColorFormat::R8Uint},
                   {"R8Sint", ColorFormat::R8Sint}, {"R8Srgb", ColorFormat::R8Srgb}, {"R16Unorm", ColorFormat::R16Unorm},
                   {"D16Unorm", ColorFormat::D16Unorm}, {"R16Snorm", ColorFormat::R16Snorm},
                   {"R16Uscaled", ColorFormat::R16Uscaled}, {"R16Sscaled", ColorFormat::R16Sscaled},
                   {"R16Uint", ColorFormat::R16Uint}, {"R16Sint", ColorFormat::R16Sint}, {"R16Sfloat", ColorFormat::R16Sfloat},
                   {"D32Sfloat", ColorFormat::D32Sfloat}, {"R32Sfloat", ColorFormat::R32Sfloat},
                   {"R32Uint", ColorFormat::R32Uint}, {"R32Sint", ColorFormat::R32Sint}, {"R8G8Unorm", ColorFormat::R8G8Unorm},
                   {"R8G8Snorm", ColorFormat::R8G8Snorm}, {"R8G8Uscaled", ColorFormat::R8G8Uscaled},
                   {"R8G8Sscaled", ColorFormat::R8G8Sscaled}, {"R8G8Uint", ColorFormat::R8G8Uint},
                   {"R8G8Sint", ColorFormat::R8G8Sint}, {"R8G8Srgb", ColorFormat::R8G8Srgb},
                   {"R16G16Unorm", ColorFormat::R16G16Unorm}, {"R16G16Snorm", ColorFormat::R16G16Snorm},
                   {"R16G16Uscaled", ColorFormat::R16G16Uscaled}, {"R16G16Sscaled", ColorFormat::R16G16Sscaled},
                   {"R16G16Uint", ColorFormat::R16G16Uint}, {"R16G16Sint", ColorFormat::R16G16Sint},
                   {"R16G16Sfloat", ColorFormat::R16G16Sfloat}, {"D16UnormS8Uint", ColorFormat::D16UnormS8Uint},
                   {"D24UnormS8Uint", ColorFormat::D24UnormS8Uint}, {"R32G32Uint", ColorFormat::R32G32Uint},
                   {"R32G32Sint", ColorFormat::R32G32Sint}, {"R32G32Sfloat", ColorFormat::R32G32Sfloat},
                   {"D32SfloatS8Uint", ColorFormat::D32SfloatS8Uint}, {"R8G8B8Unorm", ColorFormat::R8G8B8Unorm},
                   {"B8G8R8Unorm", ColorFormat::B8G8R8Unorm}, {"R8G8B8Snorm", ColorFormat::R8G8B8Snorm},
                   {"B8G8R8Snorm", ColorFormat::B8G8R8Snorm}, {"R8G8B8Uscaled", ColorFormat::R8G8B8Uscaled},
                   {"B8G8R8Uscaled", ColorFormat::B8G8R8Uscaled}, {"R8G8B8Sscaled", ColorFormat::R8G8B8Sscaled},
                   {"B8G8R8Sscaled", ColorFormat::B8G8R8Sscaled}, {"R8G8B8Uint", ColorFormat::R8G8B8Uint},
                   {"B8G8R8Uint", ColorFormat::B8G8R8Uint}, {"R8G8B8Sint", ColorFormat::R8G8B8Sint},
                   {"B8G8R8Sint", ColorFormat::B8G8R8Sint}, {"R8G8B8Srgb", ColorFormat::R8G8B8Srgb},
                   {"B8G8R8Srgb", ColorFormat::B8G8R8Srgb}, {"R16G16B16Unorm", ColorFormat::R16G16B16Unorm},
                   {"R16G16B16Snorm", ColorFormat::R16G16B16Snorm}, {"R16G16B16Uscaled", ColorFormat::R16G16B16Uscaled},
                   {"R16G16B16Sscaled", ColorFormat::R16G16B16Sscaled}, {"R16G16B16Uint", ColorFormat::R16G16B16Uint},
                   {"R16G16B16Sint", ColorFormat::R16G16B16Sint}, {"R16G16B16Sfloat", ColorFormat::R16G16B16Sfloat},
                   {"R32G32B32Uint", ColorFormat::R32G32B32Uint}, {"R32G32B32Sint", ColorFormat::R32G32B32Sint},
                   {"R32G32B32Sfloat", ColorFormat::R32G32B32Sfloat}, {"R8G8B8A8Unorm", ColorFormat::R8G8B8A8Unorm},
                   {"B8G8R8A8Unorm", ColorFormat::B8G8R8A8Unorm}, {"R8G8B8A8Snorm", ColorFormat::R8G8B8A8Snorm},
                   {"B8G8R8A8Snorm", ColorFormat::B8G8R8A8Snorm}, {"R8G8B8A8Uscaled", ColorFormat::R8G8B8A8Uscaled},
                   {"B8G8R8A8Uscaled", ColorFormat::B8G8R8A8Uscaled}, {"R8G8B8A8Sscaled", ColorFormat::R8G8B8A8Sscaled},
                   {"B8G8R8A8Sscaled", ColorFormat::B8G8R8A8Sscaled}, {"R8G8B8A8Uint", ColorFormat::R8G8B8A8Uint},
                   {"B8G8R8A8Uint", ColorFormat::B8G8R8A8Uint}, {"R8G8B8A8Sint", ColorFormat::R8G8B8A8Sint},
                   {"B8G8R8A8Sint", ColorFormat::B8G8R8A8Sint}, {"R8G8B8A8Srgb", ColorFormat::R8G8B8A8Srgb},
                   {"B8G8R8A8Srgb", ColorFormat::B8G8R8A8Srgb}, {"R16G16B16A16Unorm", ColorFormat::R16G16B16A16Unorm},
                   {"R16G16B16A16Snorm", ColorFormat::R16G16B16A16Snorm},
                   {"R16G16B16A16Uscaled", ColorFormat::R16G16B16A16Uscaled},
                   {"R16G16B16A16Sscaled", ColorFormat::R16G16B16A16Sscaled}, {"R16G16B16A16Uint", ColorFormat::R16G16B16A16Uint},
                   {"R16G16B16A16Sint", ColorFormat::R16G16B16A16Sint}, {"R16G16B16A16Sfloat", ColorFormat::R16G16B16A16Sfloat},
                   {"R32G32B32A32Uint", ColorFormat::R32G32B32A32Uint}, {"R32G32B32A32Sint", ColorFormat::R32G32B32A32Sint},
                   {"R32G32B32A32Sfloat", ColorFormat::R32G32B32A32Sfloat});
}// namespace Engine
