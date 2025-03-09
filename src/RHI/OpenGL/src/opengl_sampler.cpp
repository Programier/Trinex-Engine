#include <Core/default_resources.hpp>
#include <Graphics/sampler.hpp>
#include <opengl_api.hpp>
#include <opengl_enums_convertor.hpp>
#include <opengl_sampler.hpp>


namespace Engine
{
	OpenGL_Sampler::OpenGL_Sampler(const Sampler* sampler)
	{
		glGenSamplers(1, &m_id);

		switch (sampler->filter)
		{
			case SamplerFilter::Trilinear:
				glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;

			case SamplerFilter::Bilinear:
				glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;

			case SamplerFilter::Point:
				glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;

			default:
				glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
		}

		glSamplerParameteri(m_id, GL_TEXTURE_WRAP_S, wrap_from(sampler->address_u));
		glSamplerParameteri(m_id, GL_TEXTURE_WRAP_T, wrap_from(sampler->address_v));
		glSamplerParameteri(m_id, GL_TEXTURE_WRAP_R, wrap_from(sampler->address_w));
		glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_MODE, compare_mode(sampler->compare_mode));
		glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_FUNC, compare_func(sampler->compare_func));

#if USING_OPENGL_CORE
		glSamplerParameterf(m_id, GL_TEXTURE_MAX_ANISOTROPY, sampler->anisotropy);
		glSamplerParameterf(m_id, GL_TEXTURE_LOD_BIAS, sampler->mip_lod_bias);
		//glSamplerParameteri(m_id, GL_TEXTURE_UNNORMALIZED_COORDINATES_ARM, sampler->unnormalized_coordinates);
#endif
		glSamplerParameterf(m_id, GL_TEXTURE_MIN_LOD, sampler->min_lod);
		glSamplerParameterf(m_id, GL_TEXTURE_MAX_LOD, sampler->max_lod);
	}

	void OpenGL_Sampler::bind(BindLocation location)
	{
		glBindSampler(location.binding, m_id);
		OPENGL_API->m_sampler_units.push_back(location.binding);
	}

	OpenGL_Sampler::~OpenGL_Sampler()
	{
		glDeleteSamplers(1, &m_id);
	}

	RHI_Sampler* OpenGL::create_sampler(const Sampler* sampler)
	{
		return new OpenGL_Sampler(sampler);
	}
}// namespace Engine
