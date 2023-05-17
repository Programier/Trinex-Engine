#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum class TextureType
    {
        Texture2D,
        TextureCubeMap,
    };

    enum class PixelType : EnumerateType
    {
        RGB          = 0,
        RGBA         = 1,
        Red          = 2,
        Depth        = 3,
        Stencil      = 4,
        DepthStencil = 5,
    };

    enum class PixelComponentType : EnumerateType
    {
        UnsignedByte      = 0,
        Float             = 1,
        Depth16           = 2,
        Stencil8          = 3,
        Depth32F_Stencil8 = 4,
        Depth32F          = 5,
        Depth24_Stencil8  = 6,
        Float16           = 7,
    };

    enum class CompareMode
    {
        None,
        RefToTexture
    };

    enum class TextureFilter : uint_t
    {
        Nearest = 0,
        Linear  = 1,
    };

    enum class SamplerMipmapMode : uint_t
    {
        Nearest = 0,
        Linear  = 1,
    };


    enum class SwizzleValue : uint_t
    {
        Identity = 0,
        Zero     = 1,
        One      = 2,
        R        = 3,
        G        = 4,
        B        = 5,
        A        = 6
    };

    struct SwizzleRGBA {
        SwizzleValue R = SwizzleValue::Identity;
        SwizzleValue G = SwizzleValue::Identity;
        SwizzleValue B = SwizzleValue::Identity;
        SwizzleValue A = SwizzleValue::Identity;
    };

    enum class WrapValue : uint_t
    {
        Repeat            = 0,
        ClampToEdge       = 1,
        ClampToBorder     = 2,
        MirroredRepeat    = 3,
        MirrorClampToEdge = 4,
    };


    //    struct TextureParams {
    //        PixelType pixel_type                    = PixelType::RGBA;
    //        PixelComponentType pixel_component_type = PixelComponentType::Float;
    //        MipMapLevel mipmap_count               = 1;
    //        Size2D base_size;
    //    };


    enum class TextureCubeMapFace : byte
    {
        Front = 0,
        Back  = 1,
        Up    = 2,
        Down  = 3,
        Left  = 4,
        Right = 5
    };

    struct TextureCreateInfo {
        Size2D size                             = {1, 1};
        MipMapLevel base_mip_level              = 0;
        MipMapLevel mipmap_count                = 1;
        PixelType pixel_type                    = PixelType::RGBA;
        PixelComponentType pixel_component_type = PixelComponentType::UnsignedByte;
        TextureFilter min_filter                = TextureFilter::Linear;
        TextureFilter mag_filter                = TextureFilter::Linear;
        SamplerMipmapMode mipmap_mode           = SamplerMipmapMode::Linear;
        WrapValue wrap_s                        = WrapValue::Repeat;
        WrapValue wrap_t                        = WrapValue::Repeat;
        WrapValue wrap_r                        = WrapValue::Repeat;
        float mip_lod_bias                      = 0.0;
        float anisotropy                        = 1.0;
        CompareMode compare_mode                = CompareMode::None;
        float min_lod                           = -1000.0;
        float max_lod                           = 1000.0;
        CompareFunc compare_func                = CompareFunc::Always;
        SwizzleRGBA swizzle;
    };

}// namespace Engine
