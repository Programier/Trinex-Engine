#pragma once
#include <api.hpp>
#include <opengl_headers.hpp>


namespace Engine
{

    struct OpenGL_SamplerCreateInfo
    {
        GLuint mag_filter;
        GLuint min_filter;

        GLuint wrap_s;
        GLuint wrap_t;
        GLuint wrap_r;

        GLuint compare_mode;
        GLuint compare_func;

        float anisotropy;
        float mip_lod_bias;
        float min_lod;
        float max_lod;

        OpenGL_SamplerCreateInfo();
        OpenGL_SamplerCreateInfo(const SamplerCreateInfo&);
    };

    struct OpenGL_Sampler : RHI::RHI_Sampler
    {
        GLuint _M_sampler;

        OpenGL_Sampler();

        OpenGL_Sampler& create(const OpenGL_SamplerCreateInfo&);
        OpenGL_Sampler& destroy();
        void bind(BindingIndex location, BindingIndex binding) override;
        ~OpenGL_Sampler();
    };
}
