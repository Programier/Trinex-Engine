#include <Graphics/texture.hpp>
#include <opengl_api.hpp>
#include <opengl_texture.hpp>

namespace Engine
{
    void OpenGL_Texture::bind(BindLocation location)
    {
        if (location.set > 0)
        {
            throw EngineException("Cannot bind texture to set > 0");
        }

        glActiveTexture(GL_TEXTURE0 + location.binding);
        glBindTexture(_M_type, _M_id);
    }

    void OpenGL_Texture::generate_mipmap()
    {
        glBindTexture(_M_type, _M_id);
        glGenerateMipmap(_M_type);
        glBindTexture(_M_type, 0);
    }

    void OpenGL_Texture::bind_combined(RHI_Sampler* sampler, BindLocation location)
    {
        bind(location);
        sampler->bind(location);
    }

    void OpenGL_Texture::update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap,
                                           const byte* data)
    {
        glBindTexture(_M_type, _M_id);
        glTexSubImage2D(_M_type, static_cast<GLint>(mipmap), static_cast<GLsizei>(offset.x),
                        static_cast<GLsizei>(offset.y), static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
                        _M_format._M_format, _M_format._M_type, data);
        glBindTexture(_M_type, 0);
    }


    static GLuint texture_type(const Texture* texture)
    {
        switch (texture->type())
        {
            case TextureType::Texture2D:
                return GL_TEXTURE_2D;
            case TextureType::TextureCubeMap:
                return GL_TEXTURE_CUBE_MAP;
            default:
                return 0;
        }
    }

    static GLuint swizzle_value(SwizzleValue value, GLuint _default)
    {
        switch (value)
        {
            case SwizzleValue::R:
                return GL_BLUE;
            case SwizzleValue::G:
                return GL_GREEN;
            case SwizzleValue::B:
                return GL_BLUE;
            case SwizzleValue::A:
                return GL_ALPHA;
            case SwizzleValue::One:
                return GL_ONE;
            case SwizzleValue::Zero:
                return GL_ZERO;

            case SwizzleValue::Identity:
                return _default;
            default:
                break;
        }

        return _default;
    }

    void OpenGL_Texture::init(const Texture* texture, const byte* data)
    {
        _M_format = color_format_from_engine_format(texture->format);
        _M_type   = texture_type(texture);
        _M_size   = texture->size;

        glGenTextures(1, &_M_id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(_M_type, _M_id);
        glTexParameteri(_M_type, GL_TEXTURE_BASE_LEVEL, texture->base_mip_level);
        glTexParameteri(_M_type, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture->mipmap_count - 1));
        glTexParameteri(_M_type, GL_TEXTURE_SWIZZLE_R, swizzle_value(texture->swizzle.R, GL_RED));
        glTexParameteri(_M_type, GL_TEXTURE_SWIZZLE_G, swizzle_value(texture->swizzle.G, GL_GREEN));
        glTexParameteri(_M_type, GL_TEXTURE_SWIZZLE_B, swizzle_value(texture->swizzle.B, GL_BLUE));
        glTexParameteri(_M_type, GL_TEXTURE_SWIZZLE_A, swizzle_value(texture->swizzle.A, GL_ALPHA));

        glTexImage2D(_M_type, 0, _M_format._M_internal_format, _M_size.x, _M_size.y, GL_FALSE, _M_format._M_format,
                     _M_format._M_type, data);

        glBindTexture(_M_type, 0);
    }

    OpenGL_Texture::~OpenGL_Texture()
    {
        if (_M_id)
        {
            glDeleteTextures(1, &_M_id);
        }
    }

    RHI_Texture* OpenGL::create_texture(const Texture* texture, const byte* data)
    {
        OpenGL_Texture* opengl_texture = new OpenGL_Texture();
        opengl_texture->init(texture, data);
        return opengl_texture;
    }

}// namespace Engine
