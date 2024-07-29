#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <imgui.h>
#include <opengl_api.hpp>
#include <opengl_render_target.hpp>
#include <opengl_sampler.hpp>
#include <opengl_texture.hpp>

namespace Engine
{
    void OpenGL_Texture::bind(BindLocation location)
    {
        glActiveTexture(GL_TEXTURE0 + location.binding);
        glBindTexture(m_type, m_id);
    }

    void OpenGL_Texture::bind_combined(RHI_Sampler* sampler, BindLocation location)
    {
        bind(location);
        if (sampler)
        {
            reinterpret_cast<OpenGL_Sampler*>(sampler)->bind(location);
        }
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

    void OpenGL_Texture::init(const Texture2D* texture)
    {
        trinex_always_check(texture->mipmap_count() > 0, "Cannot create texture with zero mips!");
        m_format = color_format_from_engine_format(texture->format());
        m_type   = texture_type(texture);
        m_size   = texture->size(0);

        glGenTextures(1, &m_id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(m_type, m_id);
        glTexParameteri(m_type, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(m_type, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture->mipmap_count() - 1));
        glTexParameteri(m_type, GL_TEXTURE_SWIZZLE_R, swizzle_value(texture->swizzle_r, GL_RED));
        glTexParameteri(m_type, GL_TEXTURE_SWIZZLE_G, swizzle_value(texture->swizzle_g, GL_GREEN));
        glTexParameteri(m_type, GL_TEXTURE_SWIZZLE_B, swizzle_value(texture->swizzle_b, GL_BLUE));
        glTexParameteri(m_type, GL_TEXTURE_SWIZZLE_A, swizzle_value(texture->swizzle_a, GL_ALPHA));

        if (m_format.m_format == 0)
        {
            for (MipMapLevel i = 0, count = texture->mipmap_count(); i < count; ++i)
            {
                if (auto mip = texture->mip(i))
                {
                    glCompressedTexImage2D(m_type, 0, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, mip->data.size(),
                                           mip->data.data());
                }
                else
                {
                    glCompressedTexImage2D(m_type, 0, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, 0, nullptr);
                }
            }
        }
        else
        {
            for (MipMapLevel i = 0, count = texture->mipmap_count(); i < count; ++i)
            {
                if (auto mip = texture->mip(i))
                {
                    glTexImage2D(m_type, 0, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, m_format.m_format,
                                 m_format.m_type, mip->data.data());
                }
                else
                {
                    glTexImage2D(m_type, 0, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, m_format.m_format,
                                 m_format.m_type, nullptr);
                }
            }
        }

        glBindTexture(m_type, 0);
    }


    void OpenGL_Texture::clear_color(const Color& color)
    {}

    void OpenGL_Texture::clear_depth_stencil(float depth, byte stencil)
    {}

    OpenGL_Texture::~OpenGL_Texture()
    {
        if (m_id)
        {
            glDeleteTextures(1, &m_id);
        }
    }

    void OpenGL_RenderSurface::clear_color(const Color& color)
    {
        if (!is_in<GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL>(m_format.m_format))
        {
            OpenGL_RenderSurface* texture[] = {this};
            OPENGL_API->bind_render_target(texture, nullptr);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClearColor(color.r, color.g, color.b, color.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    void OpenGL_RenderSurface::clear_depth_stencil(float depth, byte stencil)
    {
        if (is_in<GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL>(m_format.m_format))
        {
            glDepthMask(GL_TRUE);
            OPENGL_API->bind_render_target({}, this);

            glClearDepthf(depth);
            GLbitfield field = GL_DEPTH_BUFFER_BIT;

            if (m_format.m_format == GL_DEPTH_STENCIL)
            {
                glClearStencil(stencil);
                glStencilMask(255);
                field |= GL_STENCIL_BUFFER_BIT;
            }

            glClear(field);
        }
    }

    OpenGL_RenderSurface::~OpenGL_RenderSurface()
    {
        while (!m_render_targets.empty())
        {
            OpenGL_RenderTarget* target = *m_render_targets.begin();
            delete target;
        }
    }

    RHI_Texture* OpenGL::create_texture_2d(const Texture2D* texture)
    {
        OpenGL_Texture* opengl_texture = new OpenGL_Texture();
        opengl_texture->init(texture);
        return opengl_texture;
    }

    RHI_Texture* OpenGL::create_render_surface(const RenderSurface* surface)
    {
        OpenGL_RenderSurface* opengl_texture = new OpenGL_RenderSurface();
        opengl_texture->init(surface);
        return opengl_texture;
    }
}// namespace Engine


GLuint get_opengl_texture_2d_id(Engine::Texture2D* texture)
{
    return (texture ? texture : Engine::DefaultResources::Textures::default_texture)->rhi_object<Engine::OpenGL_Texture>()->m_id;
}
