#include <Graphics/basic_texture.hpp>
#include <opengl.hpp>
#include <unordered_map>

namespace Engine
{
    static const std::unordered_map<TextureType, int> OpenGL_texture_type = {
            {TextureType::Texture_2D, GL_TEXTURE_2D},
            {TextureType::Texture_2D_Array, GL_TEXTURE_2D_ARRAY},
            {TextureType::Texture_2D_MultiSample, GL_TEXTURE_2D_MULTISAMPLE},
            {TextureType::Texture_2D_MultiSample_Array, GL_TEXTURE_2D_MULTISAMPLE_ARRAY},
            {TextureType::Texture_3D, GL_TEXTURE_3D},
            {TextureType::Texture_CubeMap, GL_TEXTURE_CUBE_MAP},
            {TextureType::Texture_CubeMapArray, GL_TEXTURE_CUBE_MAP_ARRAY}};

    static const std::unordered_map<TextureParameter, int> OpenGL_texture_param = {
            {TextureParameter::Depth_Stencil_Texture_Mode, GL_DEPTH_STENCIL_TEXTURE_MODE},
            {TextureParameter::Texture_Base_Level, GL_TEXTURE_BASE_LEVEL},
            {TextureParameter::Texture_Compare_Func, GL_TEXTURE_COMPARE_FUNC},
            {TextureParameter::Texture_Compare_Mode, GL_TEXTURE_COMPARE_MODE},
            {TextureParameter::Texture_Min_Filter, GL_TEXTURE_MIN_FILTER},
            {TextureParameter::Texture_Mag_Filter, GL_TEXTURE_MAG_FILTER},
            {TextureParameter::Texture_Min_Lod, GL_TEXTURE_MIN_LOD},
            {TextureParameter::Texture_Max_Lod, GL_TEXTURE_MAX_LOD},
            {TextureParameter::Texture_Max_Level, GL_TEXTURE_MAX_LEVEL},
            {TextureParameter::Texture_Swizzle_R, GL_TEXTURE_SWIZZLE_R},
            {TextureParameter::Texture_Swizzle_G, GL_TEXTURE_SWIZZLE_G},
            {TextureParameter::Texture_Swizzle_B, GL_TEXTURE_SWIZZLE_B},
            {TextureParameter::Texture_Swizzle_A, GL_TEXTURE_SWIZZLE_A},
            {TextureParameter::Texture_Wrap_S, GL_TEXTURE_WRAP_S},
            {TextureParameter::Texture_Wrap_T, GL_TEXTURE_WRAP_T},
            {TextureParameter::Texture_Wrap_R, GL_TEXTURE_WRAP_R}};

}// namespace Engine
