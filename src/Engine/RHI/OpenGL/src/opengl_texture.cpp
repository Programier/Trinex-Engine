#include <opengl_api.hpp>
#include <opengl_color_format.hpp>
#include <opengl_texture.hpp>
#include <opengl_types.hpp>


namespace Engine
{
    static GLint get_swizzle_value(SwizzleValue value, GLint component = 0)
    {
        GLint result = get_type(value);
        if (result != 0)
            return result;
        return component;
    }

    OpenGL_Texture& OpenGL_Texture::create_info(const TextureCreateInfo& info, TextureType type, const byte* data)
    {
        _M_format       = OpenGL_ColorFormat::from(info.format);
        _M_texture_type = get_type(type);
        size.width      = static_cast<GLsizei>(info.size.x);
        size.height     = static_cast<GLsizei>(info.size.y);

        glGenTextures(1, &_M_texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(_M_texture_type, _M_texture);
        glTexParameteri(_M_texture_type, GL_TEXTURE_BASE_LEVEL, info.base_mip_level);
        glTexParameteri(_M_texture_type, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(info.mipmap_count - 1));
        glTexParameteri(_M_texture_type, GL_TEXTURE_SWIZZLE_R, get_swizzle_value(info.swizzle.R, GL_RED));
        glTexParameteri(_M_texture_type, GL_TEXTURE_SWIZZLE_G, get_swizzle_value(info.swizzle.G, GL_GREEN));
        glTexParameteri(_M_texture_type, GL_TEXTURE_SWIZZLE_B, get_swizzle_value(info.swizzle.B, GL_BLUE));
        glTexParameteri(_M_texture_type, GL_TEXTURE_SWIZZLE_A, get_swizzle_value(info.swizzle.A, GL_ALPHA));

        glTexImage2D(_M_texture_type, 0, _M_format.internal_format, size.width, size.height, GL_FALSE, _M_format.format,
                     _M_format.type, data);

        glBindTexture(_M_texture_type, 0);
        return *this;
    }

    void OpenGL_Texture::bind(BindingIndex binding, BindingIndex set)
    {
        glActiveTexture(GL_TEXTURE0 + binding);
        glBindTexture(_M_texture_type, _M_texture);
    }

    void OpenGL_Texture::generate_mipmap()
    {
        glBindTexture(_M_texture_type, _M_texture);
        glGenerateMipmap(_M_texture_type);
        glBindTexture(_M_texture_type, 0);
    }

    void OpenGL_Texture::bind_combined(RHI_Sampler* sampler, BindingIndex binding, BindingIndex set)
    {
        bind(binding, set);
        sampler->bind(binding, set);
    }

    void OpenGL_Texture::update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap,
                                           const byte* data)
    {
        glBindTexture(_M_texture_type, _M_texture);
        glTexSubImage2D(_M_texture_type, static_cast<GLint>(mipmap), static_cast<GLsizei>(offset.x),
                        static_cast<GLsizei>(offset.y), static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
                        _M_format.format, _M_format.type, data);
        glBindTexture(_M_texture_type, 0);
    }


    OpenGL_Texture& OpenGL_Texture::destroy()
    {
        if (_M_texture)
        {
            glDeleteTextures(1, &_M_texture);
            _M_texture = 0;
        }
        return *this;
    }

    OpenGL_Texture::~OpenGL_Texture()
    {
        destroy();
    }

    Identifier OpenGL::imgui_texture_id(const Identifier& ID)
    {
        return 0;
    }

    RHI_Texture* OpenGL::create_texture(const TextureCreateInfo& info, TextureType type, const byte* data)
    {
        return &(new OpenGL_Texture())->create_info(info, type, data);
    }

}// namespace Engine
