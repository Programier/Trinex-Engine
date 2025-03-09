#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <opengl_api.hpp>
#include <opengl_enums_convertor.hpp>
#include <opengl_render_target.hpp>
#include <opengl_sampler.hpp>
#include <opengl_texture.hpp>

namespace Engine
{
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

	void OpenGL_Texture::init(const Texture2D* texture)
	{
		trinex_always_check(texture->mips.size() > 0, "Cannot create texture with zero mips!");
		m_format = color_format_from_engine_format(texture->format);
		m_type   = texture_type(texture);
		m_size   = texture->size(0);

		glGenTextures(1, &m_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindTexture(m_type, m_id);
		glTexParameteri(m_type, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(m_type, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture->mips.size() - 1));

		if (m_format.m_format == 0)
		{
			for (MipMapLevel i = 0, count = texture->mips.size(); i < count; ++i)
			{
				auto& mip = texture->mips[i];

				glCompressedTexImage2D(m_type, i, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, mip.data.size(),
									   mip.data.data());
			}
		}
		else
		{
			for (MipMapLevel i = 0, count = texture->mips.size(); i < count; ++i)
			{
				auto mip = texture->mips[i];

				glTexImage2D(m_type, i, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, m_format.m_format,
							 m_format.m_type, mip.data.data());
			}
		}

		glBindTexture(m_type, 0);
	}

	RHI_ShaderResourceView* OpenGL_Texture::create_srv()
	{
		return new OpenGL_TextureSRV(this);
	}

	RHI_UnorderedAccessView* OpenGL_Texture::create_uav()
	{
		return new OpenGL_TextureUAV(this);
	}

	OpenGL_Texture::~OpenGL_Texture()
	{
		if (m_id)
		{
			glDeleteTextures(1, &m_id);
		}
	}

	OpenGL_TextureSRV::OpenGL_TextureSRV(OpenGL_Texture* texture) : m_texture(texture)
	{
		texture->add_reference();
	}

	void OpenGL_TextureSRV::bind(BindLocation location)
	{
		glActiveTexture(GL_TEXTURE0 + location.binding);
		glBindTexture(m_texture->m_type, m_texture->m_id);
	}

	void OpenGL_TextureSRV::bind_combined(byte location, struct RHI_Sampler* sampler)
	{
		bind(location);

		if (sampler)
		{
			reinterpret_cast<OpenGL_Sampler*>(sampler)->bind(location);
		}
	}

	OpenGL_TextureSRV::~OpenGL_TextureSRV()
	{
		m_texture->release();
	}

	OpenGL_TextureUAV::OpenGL_TextureUAV(OpenGL_Texture* texture) : m_texture(texture)
	{
		texture->add_reference();
	}

	void OpenGL_TextureUAV::bind(BindLocation location) {}

	OpenGL_TextureUAV::~OpenGL_TextureUAV()
	{
		m_texture->release();
	}

	RHI_Texture* OpenGL::create_texture_2d(const Texture2D* texture)
	{
		OpenGL_Texture* opengl_texture = new OpenGL_Texture();
		opengl_texture->init(texture);
		return opengl_texture;
	}
}// namespace Engine
