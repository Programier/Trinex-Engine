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
        void bind_combined(RHI_Sampler* sampler, BindLocation location) override;

        void init(const Texture2D* texture);

        ~OpenGL_Texture();
    };
}// namespace Engine
