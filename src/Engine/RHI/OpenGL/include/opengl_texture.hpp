#pragma once
#include <Graphics/rhi.hpp>
#include <opengl_color_format.hpp>
#include <opengl_headers.hpp>


namespace Engine
{
    struct OpenGL_Texture : public RHI_Texture {
        OpenGL_ColorInfo m_format;
        GLuint m_type = 0;
        GLuint m_id   = 0;
        Size2D m_size;

        void bind(BindLocation location) override;
        void generate_mipmap() override;
        void bind_combined(RHI_Sampler* sampler, BindLocation location) override;
        void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap,
                               const byte* data) override;

        void init(const Texture* texture, const byte* data, size_t size);

        ~OpenGL_Texture();
    };
}// namespace Engine
