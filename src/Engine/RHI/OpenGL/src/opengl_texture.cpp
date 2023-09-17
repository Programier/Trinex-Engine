#include "Core/engine_types.hpp"
#include "Core/texture_types.hpp"
#include <opengl_api.hpp>
#include <opengl_color_format.hpp>
#include <opengl_texture.hpp>
#include <opengl_types.hpp>


#ifndef GL_TEXTURE_LOD_BIAS
#define GL_TEXTURE_LOD_BIAS 0x8501
#endif


namespace Engine
{
    implement_opengl_instance_cpp(OpenGL_Texture);

    OpenGL_Texture::~OpenGL_Texture()
    {
        if (_M_instance_id)
        {
            glDeleteTextures(1, &_M_instance_id);
        }
    }

    OpenGL& OpenGL::create_texture(Identifier& ID, const TextureCreateInfo& info, TextureType type)
    {
        OpenGL_Texture* texture  = new OpenGL_Texture();
        texture->_M_texture_type = get_type(type);
        ID                       = texture->ID();
        texture->_M_width        = static_cast<GLsizei>(info.size.x);
        texture->_M_height       = static_cast<GLsizei>(info.size.y);
        texture->_M_format       = OpenGL_ColorFormat::from(info.format);
        glGenTextures(1, &texture->_M_instance_id);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(texture->_M_texture_type, texture->_M_instance_id);

        glTexImage2D(texture->_M_texture_type, 0, texture->_M_format.internal_format, texture->size().width,
                     texture->size().height, GL_FALSE, texture->_M_format.format, texture->_M_format.type, nullptr);

        texture->_M_use_sampler_mode_linear = info.mipmap_mode == SamplerMipmapMode::Linear;

        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_MAX_LEVEL),
                                static_cast<GLint>(info.mipmap_count - 1));

        API->base_level_texture(ID, info.base_mip_level)
                .min_filter_texture(ID, info.min_filter)
                .mag_filter_texture(ID, info.mag_filter)
                .compare_func_texture(ID, info.compare_func)
                .compare_mode_texture(ID, info.compare_mode)
                .sample_mipmap_mode_texture(ID, info.mipmap_mode)
                .wrap_s_texture(ID, info.wrap_s)
                .wrap_t_texture(ID, info.wrap_t)
                .wrap_r_texture(ID, info.wrap_r)
                .lod_bias_texture(ID, info.mip_lod_bias)
                .anisotropic_filtering_texture(ID, info.anisotropy)
                .swizzle_texture(ID, info.swizzle)
                .min_lod_level_texture(ID, info.min_lod)
                .max_lod_level_texture(ID, info.max_lod)
                .generate_texture_mipmap(ID);
        return *this;
    }

    OpenGL& OpenGL::internal_bind_texture(OpenGL_Texture* texture)
    {
        static GLuint last_type = 0;
        if (texture)
        {
            glBindTexture(texture->_M_texture_type, texture->_M_instance_id);
            last_type = texture->_M_texture_type;
        }
        else
        {
            glBindTexture(last_type, 0);
        }
        return *this;
    }

    OpenGL& OpenGL::bind_texture(const Identifier& ID, TextureBindIndex index)
    {
        glActiveTexture(GL_TEXTURE0 + index);
        internal_bind_texture(GET_TYPE(OpenGL_Texture, ID));
        return *this;
    }


    template<typename ReturnType = GLint>
    ReturnType get_parameteri(OpenGL_Texture* texture, GLenum name)
    {
        GLint result = 0;
        glGetTexParameteriv(texture->_M_texture_type, name, &result);
        return static_cast<ReturnType>(result);
    }

    template<typename ReturnType = GLfloat>
    ReturnType get_parameterf(OpenGL_Texture* texture, GLenum name)
    {
        GLfloat result = 0;
        glGetTexParameterfv(texture->_M_texture_type, name, &result);
        return static_cast<ReturnType>(result);
    }

    MipMapLevel OpenGL::base_level_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        return texture->apply_function(get_parameteri<MipMapLevel>, texture,
                                       static_cast<GLenum>(GL_TEXTURE_BASE_LEVEL));
    }

    OpenGL& OpenGL::base_level_texture(const Identifier& ID, MipMapLevel level)
    {
        auto texture = GET_TYPE(OpenGL_Texture, ID);
        texture->apply_function<void, GLenum, GLenum, GLint>(glTexParameteri, texture->_M_texture_type,
                                                             GL_TEXTURE_BASE_LEVEL, level);
        return *this;
    }

    CompareFunc OpenGL::compare_func_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        auto result =
                texture->apply_function(get_parameteri<GLint>, texture, static_cast<GLenum>(GL_TEXTURE_COMPARE_FUNC));
        return opengl_type_to_engine_type<CompareFunc>(_M_compare_funcs, result);
    }

    OpenGL& OpenGL::compare_func_texture(const Identifier& ID, CompareFunc func)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_COMPARE_FUNC),
                                get_type(func));
        return *this;
    }

    CompareMode OpenGL::compare_mode_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        auto result =
                texture->apply_function(get_parameteri<GLint>, texture, static_cast<GLenum>(GL_TEXTURE_COMPARE_MODE));
        return opengl_type_to_engine_type<CompareMode>(_M_compare_modes, result);
    }

    OpenGL& OpenGL::compare_mode_texture(const Identifier& ID, CompareMode mode)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_COMPARE_MODE),
                                get_type(mode));
        return *this;
    }

    TextureFilter OpenGL::min_filter_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        GLint result =
                texture->apply_function(get_parameteri<GLint>, texture, static_cast<GLenum>(GL_TEXTURE_MIN_FILTER));
        if (result > GL_LINEAR)
            result -= ((texture->_M_use_sampler_mode_linear * 2) + 0x100);

        return opengl_type_to_engine_type<TextureFilter>(_M_texture_filters, result);
    }

    TextureFilter OpenGL::mag_filter_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);

        GLint result =
                texture->apply_function(get_parameteri<GLint>, texture, static_cast<GLenum>(GL_TEXTURE_MAG_FILTER));
        return opengl_type_to_engine_type<TextureFilter>(_M_texture_filters, result);
    }

    OpenGL& OpenGL::min_filter_texture(const Identifier& ID, TextureFilter filter)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);

        GLint value = get_type(filter) + 0x100 + (texture->_M_use_sampler_mode_linear * 2);
        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_MIN_FILTER),
                                value);
        return *this;
    }

    OpenGL& OpenGL::mag_filter_texture(const Identifier& ID, TextureFilter filter)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        GLint value             = get_type(filter);
        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_MAG_FILTER),
                                value);
        return *this;
    }

    OpenGL& OpenGL::min_lod_level_texture(const Identifier& ID, LodLevel level)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        texture->apply_function(glTexParameterf, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_MIN_LOD),
                                level);
        return *this;
    }

    OpenGL& OpenGL::max_lod_level_texture(const Identifier& ID, LodLevel level)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        texture->apply_function(glTexParameterf, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_MAX_LOD),
                                level);
        return *this;
    }

    LodLevel OpenGL::min_lod_level_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        return texture->apply_function(get_parameterf<LodLevel>, texture, static_cast<GLenum>(GL_TEXTURE_MIN_LOD));
    }

    LodLevel OpenGL::max_lod_level_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        return texture->apply_function(get_parameterf<LodLevel>, texture, static_cast<GLenum>(GL_TEXTURE_MAX_LOD));
    }

    MipMapLevel OpenGL::max_mipmap_level_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        return texture->apply_function(get_parameteri<MipMapLevel>, texture, static_cast<GLenum>(GL_TEXTURE_MAX_LEVEL));
    }

    static GLint get_swizzle_value(SwizzleValue value, GLint component = 0)
    {
        GLint result = get_type(value);
        if (result != 0)
            return result;
        return component;
    }

    OpenGL& OpenGL::swizzle_texture(const Identifier& ID, const SwizzleRGBA& swizzle)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);

        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_SWIZZLE_R),
                                get_swizzle_value(swizzle.R, GL_RED));
        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_SWIZZLE_G),
                                get_swizzle_value(swizzle.G, GL_GREEN));
        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_SWIZZLE_B),
                                get_swizzle_value(swizzle.B, GL_BLUE));
        texture->apply_function(glTexParameteri, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_SWIZZLE_A),
                                get_swizzle_value(swizzle.A, GL_ALPHA));
        return *this;
    }

    SwizzleRGBA OpenGL::swizzle_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);

        GLint R = texture->apply_function(get_parameteri<GLint>, texture, static_cast<GLenum>(GL_TEXTURE_SWIZZLE_R));
        GLint G = texture->apply_function(get_parameteri<GLint>, texture, static_cast<GLenum>(GL_TEXTURE_SWIZZLE_G));
        GLint B = texture->apply_function(get_parameteri<GLint>, texture, static_cast<GLenum>(GL_TEXTURE_SWIZZLE_B));
        GLint A = texture->apply_function(get_parameteri<GLint>, texture, static_cast<GLenum>(GL_TEXTURE_SWIZZLE_A));

        SwizzleRGBA result;

        result.R = opengl_type_to_engine_type<SwizzleValue>(_M_swizzle_values, R);
        result.G = opengl_type_to_engine_type<SwizzleValue>(_M_swizzle_values, G);
        result.B = opengl_type_to_engine_type<SwizzleValue>(_M_swizzle_values, B);
        result.A = opengl_type_to_engine_type<SwizzleValue>(_M_swizzle_values, A);

        return result;
    }

    static OpenGL& texture_wrap(Identifier ID, WrapValue value, GLenum param)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        GLint wrap              = get_type(value);
        texture->apply_function(glTexParameteri, texture->_M_texture_type, param, wrap);
        return *API;
    }

    OpenGL& OpenGL::wrap_s_texture(const Identifier& ID, const WrapValue& value)
    {
        return texture_wrap(ID, value, GL_TEXTURE_WRAP_S);
    }

    OpenGL& OpenGL::wrap_t_texture(const Identifier& ID, const WrapValue& value)
    {
        return texture_wrap(ID, value, GL_TEXTURE_WRAP_T);
    }

    OpenGL& OpenGL::wrap_r_texture(const Identifier& ID, const WrapValue& value)
    {
        return texture_wrap(ID, value, GL_TEXTURE_WRAP_R);
    }

    static WrapValue get_wrap(Identifier ID, GLenum param)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        GLint result            = texture->apply_function(get_parameteri<GLint>, texture, param);
        return opengl_type_to_engine_type<WrapValue>(_M_wrap_values, result);
    }

    WrapValue OpenGL::wrap_s_texture(const Identifier& ID)
    {
        return get_wrap(ID, GL_TEXTURE_WRAP_S);
    }

    WrapValue OpenGL::wrap_t_texture(const Identifier& ID)
    {
        return get_wrap(ID, GL_TEXTURE_WRAP_T);
    }

    WrapValue OpenGL::wrap_r_texture(const Identifier& ID)
    {
        return get_wrap(ID, GL_TEXTURE_WRAP_R);
    }

    OpenGL& OpenGL::anisotropic_filtering_texture(const Identifier& ID, float value)
    {
        if (_M_support_anisotropy)
        {
            value                   = glm::max(1.0f, glm::min(value, max_anisotropic_filtering()));
            OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
            texture->apply_function(glTexParameterf, texture->_M_texture_type,
                                    static_cast<GLenum>(GL_TEXTURE_MAX_ANISOTROPY_EXT), static_cast<GLfloat>(value));
        }

        return *this;
    }

    float OpenGL::anisotropic_filtering_texture(const Identifier& ID)
    {
        if (_M_support_anisotropy)
        {
            OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
            return texture->apply_function(get_parameterf<GLfloat>, texture,
                                           static_cast<GLenum>(GL_TEXTURE_MAX_ANISOTROPY_EXT));
        }

        return 1.0f;
    }

    float OpenGL::max_anisotropic_filtering()
    {
        GLfloat result = -1.0;
        if (result == -1.0f)
        {
            if (_M_support_anisotropy)
            {
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &result);
            }
            else
            {
                result = 1.0f;
            }
        }
        return result;
    }

    OpenGL& OpenGL::texture_size(const Identifier& ID, Size2D& size, MipMapLevel level)
    {
        level                   = glm::min(level, max_mipmap_level_texture(ID));
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        uint_t w = texture->_M_width, h = texture->_M_height;

        while (level-- > 0)
        {
            w = w > 1 ? w >> 1 : 1;
            w = w > 1 ? w >> 1 : 1;
        }

        size = {static_cast<float>(w), static_cast<float>(h)};
        return *this;
    }

    OpenGL& OpenGL::generate_texture_mipmap(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        texture->apply_function(glGenerateMipmap, texture->_M_texture_type);
        return *this;
    }

    OpenGL& OpenGL::read_texture_2D_data(const Identifier&, Vector<byte>& data, MipMapLevel)
    {
        return *this;
    }

    Identifier OpenGL::imgui_texture_id(const Identifier& ID)
    {
        return GET_TYPE(OpenGL_Texture, ID)->_M_instance_id;
    }


    SamplerMipmapMode OpenGL::sample_mipmap_mode_texture(const Identifier& ID)
    {
        return GET_TYPE(OpenGL_Texture, ID)->_M_use_sampler_mode_linear ? SamplerMipmapMode::Linear
                                                                        : SamplerMipmapMode::Nearest;
    }

    OpenGL& OpenGL::sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode)
    {
        auto current_filter                                      = min_filter_texture(ID);
        GET_TYPE(OpenGL_Texture, ID)->_M_use_sampler_mode_linear = (mode == SamplerMipmapMode::Linear ? 1 : 0);
        min_filter_texture(ID, current_filter);
        return *this;
    }

    LodBias OpenGL::lod_bias_texture(const Identifier& ID)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        return texture->apply_function(get_parameterf<LodBias>, texture, static_cast<GLenum>(GL_TEXTURE_LOD_BIAS));
    }

    OpenGL& OpenGL::lod_bias_texture(const Identifier& ID, LodBias bias)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        texture->apply_function(glTexParameterf, texture->_M_texture_type, static_cast<GLenum>(GL_TEXTURE_LOD_BIAS),
                                static_cast<GLfloat>(bias));
        return *this;
    }

    LodBias OpenGL::max_lod_bias_texture()
    {
        static bool inited   = false;
        static GLfloat value = 0.0;

        if (!inited)
        {
            glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &value);
            inited = true;
        }

        return value;
    }

    OpenGL& OpenGL::update_texture_2D(const Identifier& ID, const Size2D& size, const Offset2D& offset,
                                      MipMapLevel level, const void* data)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        internal_bind_texture(texture);

        glTexSubImage2D(texture->_M_texture_type, static_cast<GLint>(level), static_cast<GLsizei>(offset.x),
                        static_cast<GLsizei>(offset.y), static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
                        texture->_M_format.format, texture->_M_format.type, data);
        return internal_bind_texture(nullptr);
    }

    OpenGL& OpenGL::cubemap_texture_update_data(const Identifier& ID, TextureCubeMapFace face, const Size2D& size,
                                                const Offset2D& offset, MipMapLevel level, void* data)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, ID);
        internal_bind_texture(texture);
        glTexSubImage2D(get_type(face), static_cast<GLint>(level), static_cast<GLsizei>(offset.x),
                        static_cast<GLsizei>(offset.y), static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
                        texture->_M_format.format, texture->_M_format.type, data);
        return internal_bind_texture(nullptr);
    }
}// namespace Engine
