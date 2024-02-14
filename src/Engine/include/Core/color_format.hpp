#pragma once
#include <Core/engine_types.hpp>


namespace Engine
{

    enum class ColorFormatMetaData
    {
        None    = 0,
        Unorm   = 1,
        Snorm   = 2,
        Uscaled = 3,
        Sscaled = 4,
        Uint    = 5,
        Sint    = 6,
        Srgb    = 7,
        Sfloat  = 8
    };

    enum class ColorFormatAspect
    {
        None         = 0,
        Color        = 1,
        Depth        = 2,
        Stencil      = 3,
        DepthStencil = 4,
    };

#define color_shift(a, b) (static_cast<EnumerateType>(a) << b)
#define make_color_format(count, size, metadata, aspect, padding)                                                      \
    (color_shift(count, 0) | color_shift(size / 8, 3) | color_shift(ColorFormatMetaData::metadata, 6) |                \
     color_shift(ColorFormatAspect::aspect, 10) | color_shift(padding, 13))

    enum class ColorFormat : EnumerateType
    {
        Undefined = 0,
        R8Unorm   = make_color_format(1, 8, Unorm, Color, 0),
        R8Snorm   = make_color_format(1, 8, Snorm, Color, 0),
        R8Uscaled = make_color_format(1, 8, Uscaled, Color, 0),
        R8Sscaled = make_color_format(1, 8, Sscaled, Color, 0),
        S8Uint    = make_color_format(1, 8, Uint, Stencil, 0),
        R8Uint    = make_color_format(1, 8, Uint, Color, 1),
        R8Sint    = make_color_format(1, 8, Sint, Color, 0),
        R8Srgb    = make_color_format(1, 8, Srgb, Color, 0),

        R16Unorm   = make_color_format(1, 16, Unorm, Color, 0),
        D16Unorm   = make_color_format(1, 16, Unorm, Color, 1),
        R16Snorm   = make_color_format(1, 16, Snorm, Color, 0),
        R16Uscaled = make_color_format(1, 16, Uscaled, Color, 0),
        R16Sscaled = make_color_format(1, 16, Sscaled, Color, 0),
        R16Uint    = make_color_format(1, 16, Uint, Color, 0),
        R16Sint    = make_color_format(1, 16, Sint, Color, 0),
        R16Sfloat  = make_color_format(1, 16, Sfloat, Color, 0),

        D32Sfloat = make_color_format(1, 32, Sfloat, Depth, 0),
        R32Sfloat = make_color_format(1, 32, Sfloat, Color, 1),
        R32Uint   = make_color_format(1, 32, Uint, Color, 0),
        R32Sint   = make_color_format(1, 32, Sint, Color, 0),

        R8G8Unorm   = make_color_format(2, 8, Unorm, Color, 0),
        R8G8Snorm   = make_color_format(2, 8, Snorm, Color, 0),
        R8G8Uscaled = make_color_format(2, 8, Uscaled, Color, 0),
        R8G8Sscaled = make_color_format(2, 8, Sscaled, Color, 0),
        R8G8Uint    = make_color_format(2, 8, Uint, Color, 0),
        R8G8Sint    = make_color_format(2, 8, Sint, Color, 0),
        R8G8Srgb    = make_color_format(2, 8, Srgb, Color, 0),

        R16G16Unorm   = make_color_format(2, 16, Unorm, Color, 0),
        R16G16Snorm   = make_color_format(2, 16, Snorm, Color, 0),
        R16G16Uscaled = make_color_format(2, 16, Uscaled, Color, 0),
        R16G16Sscaled = make_color_format(2, 16, Sscaled, Color, 0),
        R16G16Uint    = make_color_format(2, 16, Uint, Color, 0),
        R16G16Sint    = make_color_format(2, 16, Sint, Color, 0),
        R16G16Sfloat  = make_color_format(2, 16, Sfloat, Color, 0),

        D16UnormS8Uint = make_color_format(2, 24, Unorm, DepthStencil, 0),

        D24UnormS8Uint = make_color_format(2, 32, Unorm, DepthStencil, 1),
        R32G32Uint     = make_color_format(2, 32, Uint, Color, 0),
        R32G32Sint     = make_color_format(2, 32, Sint, Color, 0),
        R32G32Sfloat   = make_color_format(2, 32, Sfloat, Color, 0),

        D32SfloatS8Uint = make_color_format(2, 40, Sfloat, DepthStencil, 0),

        R8G8B8Unorm   = make_color_format(3, 8, Unorm, Color, 0),
        B8G8R8Unorm   = make_color_format(3, 8, Unorm, Color, 1),
        R8G8B8Snorm   = make_color_format(3, 8, Snorm, Color, 0),
        B8G8R8Snorm   = make_color_format(3, 8, Snorm, Color, 1),
        R8G8B8Uscaled = make_color_format(3, 8, Uscaled, Color, 0),
        B8G8R8Uscaled = make_color_format(3, 8, Uscaled, Color, 1),
        R8G8B8Sscaled = make_color_format(3, 8, Sscaled, Color, 0),
        B8G8R8Sscaled = make_color_format(3, 8, Sscaled, Color, 1),
        R8G8B8Uint    = make_color_format(3, 8, Uint, Color, 0),
        B8G8R8Uint    = make_color_format(3, 8, Uint, Color, 1),
        R8G8B8Sint    = make_color_format(3, 8, Sint, Color, 0),
        B8G8R8Sint    = make_color_format(3, 8, Sint, Color, 1),
        R8G8B8Srgb    = make_color_format(3, 8, Srgb, Color, 0),
        B8G8R8Srgb    = make_color_format(3, 8, Srgb, Color, 1),

        R16G16B16Unorm   = make_color_format(3, 16, Unorm, Color, 0),
        R16G16B16Snorm   = make_color_format(3, 16, Snorm, Color, 0),
        R16G16B16Uscaled = make_color_format(3, 16, Uscaled, Color, 0),
        R16G16B16Sscaled = make_color_format(3, 16, Sscaled, Color, 0),
        R16G16B16Uint    = make_color_format(3, 16, Uint, Color, 0),
        R16G16B16Sint    = make_color_format(3, 16, Sint, Color, 0),
        R16G16B16Sfloat  = make_color_format(3, 16, Sfloat, Color, 0),

        R32G32B32Uint   = make_color_format(3, 32, Uint, Color, 0),
        R32G32B32Sint   = make_color_format(3, 32, Sint, Color, 0),
        R32G32B32Sfloat = make_color_format(3, 32, Sfloat, Color, 0),
        R8G8B8A8Unorm   = make_color_format(4, 8, Unorm, Color, 0),
        B8G8R8A8Unorm   = make_color_format(4, 8, Unorm, Color, 1),
        R8G8B8A8Snorm   = make_color_format(4, 8, Snorm, Color, 0),
        B8G8R8A8Snorm   = make_color_format(4, 8, Snorm, Color, 1),
        R8G8B8A8Uscaled = make_color_format(4, 8, Uscaled, Color, 0),
        B8G8R8A8Uscaled = make_color_format(4, 8, Uscaled, Color, 1),
        R8G8B8A8Sscaled = make_color_format(4, 8, Sscaled, Color, 0),
        B8G8R8A8Sscaled = make_color_format(4, 8, Sscaled, Color, 1),
        R8G8B8A8Uint    = make_color_format(4, 8, Uint, Color, 0),
        B8G8R8A8Uint    = make_color_format(4, 8, Uint, Color, 1),
        R8G8B8A8Sint    = make_color_format(4, 8, Sint, Color, 0),
        B8G8R8A8Sint    = make_color_format(4, 8, Sint, Color, 1),
        R8G8B8A8Srgb    = make_color_format(4, 8, Srgb, Color, 0),
        B8G8R8A8Srgb    = make_color_format(4, 8, Srgb, Color, 1),

        R16G16B16A16Unorm   = make_color_format(4, 16, Unorm, Color, 0),
        R16G16B16A16Snorm   = make_color_format(4, 16, Snorm, Color, 0),
        R16G16B16A16Uscaled = make_color_format(4, 16, Uscaled, Color, 0),
        R16G16B16A16Sscaled = make_color_format(4, 16, Sscaled, Color, 0),
        R16G16B16A16Uint    = make_color_format(4, 16, Uint, Color, 0),
        R16G16B16A16Sint    = make_color_format(4, 16, Sint, Color, 0),
        R16G16B16A16Sfloat  = make_color_format(4, 16, Sfloat, Color, 0),

        R32G32B32A32Uint   = make_color_format(4, 32, Uint, Color, 0),
        R32G32B32A32Sint   = make_color_format(4, 32, Sint, Color, 0),
        R32G32B32A32Sfloat = make_color_format(4, 32, Sfloat, Color, 0),
    };

#undef make_color_format
#undef color_shift

    struct ENGINE_EXPORT ColorFormatFeatures {
        union
        {
            struct {
                bool is_supported : 1;
                bool support_color_attachment : 1;
                bool support_depth_stencil : 1;
            };

            byte data[1];
        };


        ColorFormatFeatures();
        bool contains(const ColorFormatFeatures& other) const;
    };

    class ENGINE_EXPORT ColorFormatInfo
    {
    private:
        EnumerateType _M_value;

    public:
        FORCE_INLINE uint_t components() const
        {
            return static_cast<uint_t>(_M_value & byte(7));
        }

        FORCE_INLINE uint_t component_size() const
        {
            return static_cast<uint_t>((_M_value >> 3) & byte(7));
        }

        FORCE_INLINE ColorFormatMetaData metadata() const
        {
            return static_cast<ColorFormatMetaData>((_M_value >> 6) & byte(15));
        }

        FORCE_INLINE ColorFormatAspect aspect() const
        {
            return static_cast<ColorFormatAspect>((_M_value >> 10) & byte(7));
        }

        FORCE_INLINE uint_t size() const
        {
            return components() * component_size();
        }

        static FORCE_INLINE ColorFormatInfo info_of(ColorFormat format)
        {
            ColorFormatInfo info;
            info._M_value = static_cast<EnumerateType>(format);
            return info;
        }

        const ColorFormatFeatures& features() const;
        static const Vector<ColorFormat>& all_formats();
    };

}// namespace Engine
