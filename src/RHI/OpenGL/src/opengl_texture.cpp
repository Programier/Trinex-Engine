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
	static FORCE_INLINE GLenum filter_of(SamplerFilter filter)
	{
		switch (filter)
		{
			case SamplerFilter::Bilinear:
			case SamplerFilter::Trilinear:
				return GL_LINEAR;

			default:
				return GL_NEAREST;
		}
	}

	template<typename T>
	static void blit_surface(T* src, T* dst, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter)
	{
		auto read  = OpenGL_RenderTarget::find_or_create(src);
		auto write = OpenGL_RenderTarget::find_or_create(dst);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, write->m_framebuffer);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, read->m_framebuffer);

		auto src_start = src_rect.pos;
		auto src_end   = src_start + src_rect.size;
		auto dst_start = dst_rect.pos;
		auto dst_end   = dst_start + dst_rect.size;

		glBlitFramebuffer(src_start.x, src_start.y, src_end.x, src_end.y, dst_start.x, dst_start.y, dst_end.x, dst_end.y,
		                  GL_COLOR_BUFFER_BIT, filter_of(filter));

		if (OPENGL_API->m_state.render_target)
		{
			OPENGL_API->m_state.render_target->bind();
		}
	}

	OpenGL_Texture::OpenGL_Texture(RHI_Object* owner) : m_owner(owner) {}

	void OpenGL_Texture::init_2D(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags)
	{
		m_format = color_format_from_engine_format(format);
		m_size   = size;
		m_flags  = flags;

		glGenTextures(1, &m_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint texture_type = type();
		glBindTexture(texture_type, m_id);
		glTexParameteri(texture_type, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(texture_type, GL_TEXTURE_MAX_LEVEL, mips - 1);

		if (m_format.m_format == 0)
		{
			for (uint32_t mip = 0; mip < mips; ++mip)
			{
				glCompressedTexImage2D(texture_type, mip, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, 0, nullptr);
				size = glm::max(Vector2u(1, 1), size / 2u);
			}
		}
		else
		{
			for (uint32_t mip = 0; mip < mips; ++mip)
			{
				glTexImage2D(texture_type, mip, m_format.m_internal_format, m_size.x, m_size.y, GL_FALSE, m_format.m_format,
				             m_format.m_type, nullptr);
				size = glm::max(Vector2u(1, 1), size / 2u);
			}
		}

		glBindTexture(texture_type, 0);
	}

	void OpenGL_Texture::update_2D(byte mip, const Rect2D& rect, const byte* data, size_t data_size)
	{
		GLuint texture_type = type();
		glBindTexture(texture_type, m_id);
		if (m_format.m_format == 0)
		{
			glCompressedTexSubImage2D(texture_type, mip, rect.pos.x, rect.pos.y, rect.size.x, rect.size.y,
			                          m_format.m_internal_format, data_size, data);
		}
		else
		{
			glTexSubImage2D(texture_type, mip, rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, m_format.m_format,
			                m_format.m_type, data);
		}
		glBindTexture(texture_type, 0);
	}

	RHI_RenderTargetView* OpenGL_Texture::create_rtv()
	{
		if (m_flags & TextureCreateFlags::RenderTarget)
			return new OpenGL_TextureRTV(this);
		return nullptr;
	}

	RHI_DepthStencilView* OpenGL_Texture::create_dsv()
	{
		if (m_flags & TextureCreateFlags::DepthStencilTarget)
			return new OpenGL_TextureDSV(this);
		return nullptr;
	}

	RHI_ShaderResourceView* OpenGL_Texture::create_srv()
	{
		if (m_flags & TextureCreateFlags::ShaderResource)
			return new OpenGL_TextureSRV(this);
		return nullptr;
	}

	RHI_UnorderedAccessView* OpenGL_Texture::create_uav()
	{
		if (m_flags & TextureCreateFlags::UnorderedAccess)
			return new OpenGL_TextureUAV(this);
		return nullptr;
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
		texture->m_owner->add_reference();
	}

	void OpenGL_TextureSRV::bind(BindLocation location)
	{
		glActiveTexture(GL_TEXTURE0 + location.binding);
		glBindTexture(m_texture->type(), m_texture->m_id);
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
		m_texture->m_owner->release();
	}

	OpenGL_TextureUAV::OpenGL_TextureUAV(OpenGL_Texture* texture) : m_texture(texture)
	{
		texture->m_owner->add_reference();
	}

	void OpenGL_TextureUAV::bind(BindLocation location)
	{
		glBindImageTexture(location, m_texture->m_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_texture->m_format.m_internal_format);
	}

	OpenGL_TextureUAV::~OpenGL_TextureUAV()
	{
		m_texture->m_owner->release();
	}

	OpenGL_TextureRTV::OpenGL_TextureRTV(OpenGL_Texture* texture) : m_texture(texture)
	{
		texture->m_owner->add_reference();
	}

	void OpenGL_TextureRTV::clear(const LinearColor& color)
	{
		auto rt = OPENGL_API->m_state.render_target;
		OPENGL_API->bind_render_target1(this);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);

		if (rt)
			rt->bind();
	}

	void OpenGL_TextureRTV::blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
	                             SamplerFilter filter)
	{
		blit_surface(static_cast<OpenGL_TextureRTV*>(surface), this, src_rect, dst_rect, filter);
	}

	OpenGL_TextureRTV::~OpenGL_TextureRTV()
	{
		m_texture->m_owner->release();
	}

	OpenGL_TextureDSV::OpenGL_TextureDSV(OpenGL_Texture* texture) : m_texture(texture)
	{
		texture->m_owner->add_reference();
	}

	void OpenGL_TextureDSV::clear(float depth, byte stencil)
	{
		glDepthMask(GL_TRUE);
		OPENGL_API->bind_render_target(nullptr, nullptr, nullptr, nullptr, this);

		glClearDepthf(depth);
		GLbitfield field = GL_DEPTH_BUFFER_BIT;

		if (m_texture->m_format.m_format == GL_DEPTH_STENCIL)
		{
			glClearStencil(stencil);
			glStencilMask(255);
			field |= GL_STENCIL_BUFFER_BIT;
		}

		glClear(field);
	}

	void OpenGL_TextureDSV::blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
	                             SamplerFilter filter)
	{
		blit_surface(static_cast<OpenGL_TextureDSV*>(surface), this, src_rect, dst_rect, filter);
	}

	OpenGL_TextureDSV::~OpenGL_TextureDSV()
	{
		m_texture->m_owner->release();
	}

	OpenGL_Texture2D::OpenGL_Texture2D(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags)
	    : m_texture(this)
	{
		m_texture.init_2D(format, size, mips, flags);
	}

	void OpenGL_Texture2D::update(byte mip, const Rect2D& rect, const byte* data, size_t data_size)
	{
		m_texture.update_2D(mip, rect, data, data_size);
	}

	RHI_Texture2D* OpenGL::create_texture_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags)
	{
		return new OpenGL_Texture2D(format, size, mips, flags);
	}
}// namespace Engine
