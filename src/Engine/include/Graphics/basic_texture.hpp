#pragma once
#include <BasicFunctional/reference_wrapper.hpp>
#include <cstddef>
#include <BasicFunctional/engine_types.hpp>


namespace Engine
{
    enum class TextureType
    {
        Texture_2D,
        Texture_2D_Array,
        Texture_2D_MultiSample,
        Texture_2D_MultiSample_Array,
        Texture_3D,
        Texture_CubeMap,
        Texture_CubeMapArray,
    };

    enum class TextureParameter
    {
        Depth_Stencil_Texture_Mode,
        Texture_Base_Level,
        Texture_Compare_Func,
        Texture_Compare_Mode,
        Texture_Min_Filter,
        Texture_Mag_Filter,
        Texture_Min_Lod,
        Texture_Max_Lod,
        Texture_Max_Level,
        Texture_Swizzle_R,
        Texture_Swizzle_G,
        Texture_Swizzle_B,
        Texture_Swizzle_A,
        Texture_Wrap_S,
        Texture_Wrap_T,
        Texture_Wrap_R
    };

    enum class TextureParamValue
    {
        DepthComponent,
        StencilIndex
    };

    class BasicTexture
    {
    protected:
        ReferenceWrapper<ObjectID> _M_ID = ReferenceWrapper<ObjectID>(0);
        TextureType _M_type;
        Size2D _M_size;

    public:
        BasicTexture();
        BasicTexture(const BasicTexture&);
        BasicTexture(Size1D width, Size1D height);
        BasicTexture& operator=(const BasicTexture& texture);

        BasicTexture& gen(const TextureType& type, Size1D width, Size1D height);
        BasicTexture& gen(const TextureType& type, Size2D size);
        ObjectID id() const;
        BasicTexture& bind(TextureBindIndex index = 0);
        const TextureType& type() const;
    };
}// namespace Engine
