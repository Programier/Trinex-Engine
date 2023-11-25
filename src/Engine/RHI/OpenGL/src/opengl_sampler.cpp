#include <Graphics/sampler.hpp>
#include <opengl_api.hpp>
#include <opengl_sampler.hpp>


namespace Engine
{

    static GLuint wrap_from(WrapValue value)
    {
        switch (value)
        {
            case WrapValue::Repeat:
                return GL_REPEAT;

            case WrapValue::MirroredRepeat:
                return GL_MIRRORED_REPEAT;

            case WrapValue::ClampToBorder:
                return GL_CLAMP_TO_BORDER;

            case WrapValue::ClampToEdge:
                return GL_CLAMP_TO_EDGE;

                /*case WrapValue::MirrorClampToEdge:
                return GL_CLAMP_TO_BORDER*/
                ;

            default:
                throw EngineException("Unsupported wrap mode");
        }
    }

    static GLuint compare_mode(CompareMode mode)
    {
        return mode == CompareMode::RefToTexture ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE;
    }

    static GLuint compare_func(CompareFunc func)
    {
        switch (func)
        {
            case CompareFunc::Always:
                return GL_ALWAYS;
            case CompareFunc::Lequal:
                return GL_LEQUAL;
            case CompareFunc::Gequal:
                return GL_GEQUAL;
            case CompareFunc::Less:
                return GL_LESS;
            case CompareFunc::Greater:
                return GL_GREATER;
            case CompareFunc::Equal:
                return GL_EQUAL;
            case CompareFunc::NotEqual:
                return GL_NOTEQUAL;
            case CompareFunc::Never:
                return GL_NEVER;
        }

        return GL_ALWAYS;
    }

    OpenGL_Sampler::OpenGL_Sampler(const Sampler* sampler)
    {
        glGenSamplers(1, &_M_id);

        switch (sampler->filter)
        {
            case SamplerFilter::Trilinear:
                glSamplerParameteri(_M_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glSamplerParameteri(_M_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;

            case SamplerFilter::Bilinear:
                glSamplerParameteri(_M_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                glSamplerParameteri(_M_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;

            case SamplerFilter::Point:
                glSamplerParameteri(_M_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glSamplerParameteri(_M_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                break;

            default:
                glSamplerParameteri(_M_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glSamplerParameteri(_M_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                break;
        }

        glSamplerParameteri(_M_id, GL_TEXTURE_WRAP_S, wrap_from(sampler->wrap_s));
        glSamplerParameteri(_M_id, GL_TEXTURE_WRAP_T, wrap_from(sampler->wrap_t));
        glSamplerParameteri(_M_id, GL_TEXTURE_WRAP_R, wrap_from(sampler->wrap_r));
        glSamplerParameteri(_M_id, GL_TEXTURE_COMPARE_MODE, compare_mode(sampler->compare_mode));
        glSamplerParameteri(_M_id, GL_TEXTURE_COMPARE_FUNC, compare_func(sampler->compare_func));

        //glSamplerParameterf(_M_id, GL_TEXTURE_MAX_ANISOTROPY_EXT, info.anisotropy);
        // glSamplerParameterf(_M_id, GL_TEXTURE_LOD_BIAS, info.mip_lod_bias);
        glSamplerParameterf(_M_id, GL_TEXTURE_MIN_LOD, sampler->min_lod);
        glSamplerParameterf(_M_id, GL_TEXTURE_MAX_LOD, sampler->max_lod);
    }

    void OpenGL_Sampler::bind(BindLocation location)
    {
        glBindSampler(location.binding, _M_id);
        OPENGL_API->_M_sampler_units.push_back(location.binding);
    }

    OpenGL_Sampler::~OpenGL_Sampler()
    {
        glDeleteSamplers(1, &_M_id);
    }

    RHI_Sampler* OpenGL::create_sampler(const Sampler* sampler)
    {
        return new OpenGL_Sampler(sampler);
    }
}// namespace Engine
