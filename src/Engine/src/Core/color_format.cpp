#include <Core/color_format.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
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
        auto it = static_color_format_features().find(static_cast<ColorFormat>(_M_value));

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
}// namespace Engine
