#include <opengl_api.hpp>
#include <opengl_color_format.hpp>
#include <opengl_render_target.hpp>
#include <opengl_sampler.hpp>
#include <opengl_surface.hpp>

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

	OpenGL_Surface& OpenGL_Surface::init(ColorFormat format, Vector2u size)
	{
		auto gl_format = color_format_from_engine_format(format);
		glGenTextures(1, &m_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, gl_format.m_internal_format, size.x, size.y, GL_FALSE, gl_format.m_format,
					 gl_format.m_type, nullptr);

		m_format = gl_format.m_format;
		return *this;
	}

	RHI_RenderTargetView* OpenGL_Surface::create_rtv()
	{
		return new OpenGL_SurfaceRTV(this);
	}

	RHI_DepthStencilView* OpenGL_Surface::create_dsv()
	{
		return new OpenGL_SurfaceDSV(this);
	}

	RHI_ShaderResourceView* OpenGL_Surface::create_srv()
	{
		return new OpenGL_SurfaceSRV(this);
	}

	RHI_UnorderedAccessView* OpenGL_Surface::create_uav()
	{
		return new OpenGL_SurfaceUAV(this);
	}

	OpenGL_Surface::~OpenGL_Surface()
	{
		glDeleteTextures(1, &m_id);
	}

	OpenGL_SurfaceSRV::OpenGL_SurfaceSRV(OpenGL_Surface* surface) : m_surface(surface)
	{
		surface->add_reference();
	}

	void OpenGL_SurfaceSRV::bind(BindLocation location)
	{
		glActiveTexture(GL_TEXTURE0 + location.binding);
		glBindTexture(GL_TEXTURE_2D, m_surface->m_id);
	}

	void OpenGL_SurfaceSRV::bind_combined(byte location, struct RHI_Sampler* sampler)
	{
		bind(location);

		if (sampler)
		{
			reinterpret_cast<OpenGL_Sampler*>(sampler)->bind(location);
		}
	}

	OpenGL_SurfaceSRV::~OpenGL_SurfaceSRV()
	{
		m_surface->release();
	}

	OpenGL_SurfaceUAV::OpenGL_SurfaceUAV(OpenGL_Surface* surface) : m_surface(surface)
	{
		surface->add_reference();
	}

	void OpenGL_SurfaceUAV::bind(BindLocation location) {}

	OpenGL_SurfaceUAV::~OpenGL_SurfaceUAV()
	{
		m_surface->release();
	}

	OpenGL_SurfaceRTV::OpenGL_SurfaceRTV(OpenGL_Surface* surface) : m_surface(surface)
	{
		surface->add_reference();
	}

	void OpenGL_SurfaceRTV::clear(const Color& color)
	{
		auto rt = OPENGL_API->m_state.render_target;
		OPENGL_API->bind_render_target1(this);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);

		if (rt)
			rt->bind();
	}

	template<typename T>
	static void blit_surface(T* src, T* dst, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter)
	{
		auto read  = OpenGL_RenderTarget::find_or_create(src);
		auto write = OpenGL_RenderTarget::find_or_create(dst);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, write->m_framebuffer);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, read->m_framebuffer);

		auto src_start = src_rect.pos;
		auto src_end   = src_start + Vector2i(src_rect.size);
		auto dst_start = dst_rect.pos;
		auto dst_end   = dst_start + Vector2i(dst_rect.size);

		glBlitFramebuffer(src_start.x, src_start.y, src_end.x, src_end.y, dst_start.x, dst_start.y, dst_end.x, dst_end.y,
						  GL_COLOR_BUFFER_BIT, filter_of(filter));

		if (OPENGL_API->m_state.render_target)
		{
			OPENGL_API->m_state.render_target->bind();
		}
	}

	void OpenGL_SurfaceRTV::blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
								 SamplerFilter filter)
	{
		blit_surface(static_cast<OpenGL_SurfaceRTV*>(surface), this, src_rect, dst_rect, filter);
	}

	OpenGL_SurfaceRTV::~OpenGL_SurfaceRTV()
	{
		while (!m_render_targets.empty())
		{
			OpenGL_RenderTarget* target = *m_render_targets.begin();
			delete target;
		}

		m_surface->release();
	}

	OpenGL_SurfaceDSV::OpenGL_SurfaceDSV(OpenGL_Surface* surface) : m_surface(surface)
	{
		surface->add_reference();
	}

	void OpenGL_SurfaceDSV::clear(float depth, byte stencil)
	{
		glDepthMask(GL_TRUE);
		OPENGL_API->bind_render_target(nullptr, nullptr, nullptr, nullptr, this);

		glClearDepthf(depth);
		GLbitfield field = GL_DEPTH_BUFFER_BIT;

		if (m_surface->m_format == GL_DEPTH_STENCIL)
		{
			glClearStencil(stencil);
			glStencilMask(255);
			field |= GL_STENCIL_BUFFER_BIT;
		}

		glClear(field);
	}

	void OpenGL_SurfaceDSV::blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
								 SamplerFilter filter)
	{
		blit_surface(static_cast<OpenGL_SurfaceDSV*>(surface), this, src_rect, dst_rect, filter);
	}

	OpenGL_SurfaceDSV::~OpenGL_SurfaceDSV()
	{
		while (!m_render_targets.empty())
		{
			OpenGL_RenderTarget* target = *m_render_targets.begin();
			delete target;
		}
		m_surface->release();
	}

	RHI_Surface* OpenGL::create_render_surface(ColorFormat format, Vector2u size)
	{
		auto surface = new OpenGL_Surface();
		surface->init(format, size);
		return surface;
	}
}// namespace Engine
