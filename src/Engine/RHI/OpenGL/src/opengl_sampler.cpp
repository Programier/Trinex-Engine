#include <Graphics/sampler.hpp>
#include <opengl_api.hpp>
#include <opengl_sampler.hpp>
#include <opengl_types.hpp>

#ifndef GL_TEXTURE_LOD_BIAS
#define GL_TEXTURE_LOD_BIAS 0x8501
#endif

namespace Engine
{

    OpenGL_SamplerCreateInfo::OpenGL_SamplerCreateInfo()
    {}

    OpenGL_SamplerCreateInfo::OpenGL_SamplerCreateInfo(const SamplerCreateInfo& info)
        : wrap_s(get_type(info.wrap_s)), wrap_t(get_type(info.wrap_t)), wrap_r(get_type(info.wrap_r)),
          compare_mode(get_type(info.compare_mode)), compare_func(get_type(info.compare_func)),
          anisotropy(info.anisotropy), mip_lod_bias(info.mip_lod_bias), min_lod(info.min_lod), max_lod(info.max_lod)
    {
        switch (info.filter)
        {
            case SamplerFilter::Trilinear:
                mag_filter = GL_LINEAR;
                min_filter = GL_LINEAR_MIPMAP_LINEAR;
                break;

            case SamplerFilter::Bilinear:
                mag_filter = GL_LINEAR;
                min_filter = GL_LINEAR_MIPMAP_NEAREST;
                break;

            case SamplerFilter::Point:
                mag_filter = GL_NEAREST;
                min_filter = GL_NEAREST_MIPMAP_NEAREST;
                break;

            default:
                mag_filter = GL_NEAREST;
                min_filter = GL_NEAREST_MIPMAP_NEAREST;
        }
    }

    OpenGL_Sampler::OpenGL_Sampler() : _M_sampler(0)
    {}

    OpenGL_Sampler& OpenGL_Sampler::create(const OpenGL_SamplerCreateInfo& info)
    {
        destroy();
        glGenSamplers(1, &_M_sampler);

        glSamplerParameteri(_M_sampler, GL_TEXTURE_MIN_FILTER, info.min_filter);
        glSamplerParameteri(_M_sampler, GL_TEXTURE_MAG_FILTER, info.mag_filter);
        glSamplerParameteri(_M_sampler, GL_TEXTURE_WRAP_S, info.wrap_s);
        glSamplerParameteri(_M_sampler, GL_TEXTURE_WRAP_T, info.wrap_t);
        glSamplerParameteri(_M_sampler, GL_TEXTURE_WRAP_R, info.wrap_r);
        glSamplerParameteri(_M_sampler, GL_TEXTURE_COMPARE_MODE, info.compare_mode);
        glSamplerParameteri(_M_sampler, GL_TEXTURE_COMPARE_FUNC, info.compare_func);

        glSamplerParameterf(_M_sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, info.anisotropy);
        glSamplerParameterf(_M_sampler, GL_TEXTURE_LOD_BIAS, info.mip_lod_bias);
        glSamplerParameterf(_M_sampler, GL_TEXTURE_MIN_LOD, info.min_lod);
        glSamplerParameterf(_M_sampler, GL_TEXTURE_MAX_LOD, info.max_lod);


        return *this;
    }

    OpenGL_Sampler& OpenGL_Sampler::destroy()
    {
        if (_M_sampler != 0)
        {
            glDeleteSamplers(1, &_M_sampler);
            _M_sampler = 0;
        }
        return *this;
    }

    void OpenGL_Sampler::bind(BindingIndex binding, BindingIndex set)
    {
        glBindSampler(binding, _M_sampler);
        API->_M_samplers.push_back(binding);
    }

    OpenGL_Sampler::~OpenGL_Sampler()
    {
        destroy();
    }


    RHI::RHI_Sampler* OpenGL::create_sampler(const SamplerCreateInfo& info)
    {
        return &(new OpenGL_Sampler())->create(info);
    }
}// namespace Engine
