#pragma once
#include <api.hpp>
#include <functional>
#include <opengl_api.hpp>
#include <opengl_color_format.hpp>
#include <opengl_object.hpp>

namespace Engine
{
    struct TextureSize {
        GLsizei width;
        GLsizei height;
    };

    struct OpenGL_Texture : public RHI::RHI_Texture {
        TextureSize size;
        OpenGL_ColorFormat _M_format;
        GLenum _M_texture_type;
        GLuint _M_texture;


        OpenGL_Texture& create_info(const TextureCreateInfo& info, TextureType type, const byte* data);

        void bind(BindingIndex binding, BindingIndex set);
        void generate_mipmap();
        void bind_combined(RHI::RHI_Sampler* sampler, BindingIndex binding, BindingIndex set);
        void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data);

        OpenGL_Texture& destroy();

        ~OpenGL_Texture();
    };
}// namespace Engine
