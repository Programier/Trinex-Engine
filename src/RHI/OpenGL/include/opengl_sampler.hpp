#pragma once
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
    struct OpenGL_Sampler : public RHI_Sampler {
        GLuint m_id;

        OpenGL_Sampler(const class Sampler* sampler);
        void bind(BindLocation location) override;
        ~OpenGL_Sampler();
    };
}// namespace Engine
