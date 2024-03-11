#include <Core/default_resources.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <imgui.h>
#include <opengl_api.hpp>
#include <opengl_sampler.hpp>
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
        glBindTexture(m_type, m_id);
    }

    void OpenGL_Texture::generate_mipmap()
    {
        glBindTexture(m_type, m_id);
        glGenerateMipmap(m_type);
        glBindTexture(m_type, 0);
    }

    void OpenGL_Texture::update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data,
                                           size_t data_size)
    {
        glBindTexture(m_type, m_id);
        if (m_format.m_format == 0)
        {
            glCompressedTexSubImage2D(m_type, 0, static_cast<GLsizei>(offset.x), static_cast<GLsizei>(offset.y),
                                      static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), m_format.m_internal_format,
                                      data_size, data);
        }
        else
        {
            glTexSubImage2D(m_type, static_cast<GLint>(mipmap), static_cast<GLsizei>(offset.x), static_cast<GLsizei>(offset.y),
                            static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), m_format.m_format, m_format.m_type, data);
        }
        glBindTexture(m_type, 0);
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

    static GLuint swizzle_value(Swizzle value, GLuint _default)
    {
        switch (value)
        {
            case Swizzle::R:
                return GL_BLUE;
            case Swizzle::G:
                return GL_GREEN;
            case Swizzle::B:
                return GL_BLUE;
            case Swizzle::A:
                return GL_ALPHA;
            case Swizzle::One:
                return GL_ONE;
            case Swizzle::Zero:
                return GL_ZERO;

            case Swizzle::Identity:
                return _default;
            default:
                break;
        }

        return _default;
    }

    void OpenGL_Texture::init(const Texture* texture, const byte* data, size_t size)
    {
        m_format = color_format_from_engine_format(texture->format);
        m_type   = texture_type(texture);
        m_size   = texture->size;

        glGenTextures(1, &m_id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(m_type, m_id);
        glTexParameteri(m_type, GL_TEXTURE_BASE_LEVEL, texture->base_mip_level);
        glTexParameteri(m_type, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture->mipmap_count - 1));
        glTexParameteri(m_type, GL_TEXTURE_SWIZZLE_R, swizzle_value(texture->swizzle_r, GL_RED));
        glTexParameteri(m_type, GL_TEXTURE_SWIZZLE_G, swizzle_value(texture->swizzle_g, GL_GREEN));
        glTexParameteri(m_type, GL_TEXTURE_SWIZZLE_B, swizzle_value(texture->swizzle_b, GL_BLUE));
        glTexParameteri(m_type, GL_TEXTURE_SWIZZLE_A, swizzle_value(texture->swizzle_a, GL_ALPHA));

        if (m_format.m_format == 0)
        {
            glCompressedTexImage2D(m_type, 0, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, size, data);
        }
        else
        {
            glTexImage2D(m_type, 0, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, m_format.m_format, m_format.m_type,
                         data);
        }

        glBindTexture(m_type, 0);
    }

    OpenGL_Texture::~OpenGL_Texture()
    {
        if (m_id)
        {
            glDeleteTextures(1, &m_id);
        }
    }

    RHI_Texture* OpenGL::create_texture(const Texture* texture, const byte* data, size_t size)
    {
        OpenGL_Texture* opengl_texture = new OpenGL_Texture();
        opengl_texture->init(texture, data, size);
        return opengl_texture;
    }
}// namespace Engine


GLuint get_opengl_texture_2d_id(Engine::Texture2D* texture)
{
    return (texture ? texture : Engine::DefaultResources::default_texture)->rhi_object<Engine::OpenGL_Texture>()->m_id;
}
