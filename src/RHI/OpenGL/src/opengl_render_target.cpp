#include <Core/colors.hpp>
#include <Core/memory.hpp>
#include <Graphics/render_surface.hpp>
#include <opengl_api.hpp>
#include <opengl_render_target.hpp>
#include <opengl_texture.hpp>

namespace Engine
{
	static GLenum get_attachment_type(GLenum type)
	{
		switch (type)
		{
			case GL_DEPTH_STENCIL:
				return GL_DEPTH_STENCIL_ATTACHMENT;
			case GL_DEPTH_COMPONENT:
				return GL_DEPTH_ATTACHMENT;
			case GL_STENCIL_INDEX:
				return GL_STENCIL_ATTACHMENT;
			default:
				return GL_COLOR_ATTACHMENT0;
		}
	}

	TreeMap<HashIndex, OpenGL_RenderTarget*> OpenGL_RenderTarget::m_render_targets;

	void OpenGL_RenderTarget::release_all()
	{
		for (auto& rt : m_render_targets)
		{
			rt.second->m_textures.clear();
			delete rt.second;
		}

		m_render_targets.clear();
	}

	OpenGL_RenderTarget* OpenGL_RenderTarget::current()
	{
		return OPENGL_API->m_state.render_target;
	}

	OpenGL_RenderTarget* OpenGL_RenderTarget::find_or_create(const Span<RenderSurface*>& color_attachments,
	                                                         RenderSurface* depth_stencil)
	{
		HashIndex hash = 0;

		for (auto& texture : color_attachments)
		{
			RHI_Object* object = texture->rhi_object<RHI_Object>();
			hash               = memory_hash_fast(&object, sizeof(object), hash);
		}

		if (depth_stencil)
		{
			RHI_Object* object = depth_stencil->rhi_object<RHI_Object>();
			hash               = memory_hash_fast(&object, sizeof(object), hash);
		}

		auto it = m_render_targets.find(hash);
		if (it != m_render_targets.end())
		{
			return it->second;
		}
		OpenGL_RenderTarget* rt = new OpenGL_RenderTarget();
		rt->m_index             = hash;
		m_render_targets[hash]  = rt;

		rt->init(color_attachments, depth_stencil);
		return rt;
	}

	OpenGL_RenderTarget* OpenGL_RenderTarget::find_or_create(const Span<OpenGL_RenderSurface*>& color_attachments,
	                                                         OpenGL_RenderSurface* depth_stencil)
	{

		HashIndex hash = 0;
		for (RHI_Object* texture : color_attachments)
		{
			hash = memory_hash_fast(&texture, sizeof(texture), hash);
		}

		if (depth_stencil)
		{
			hash = memory_hash_fast(&depth_stencil, sizeof(depth_stencil), hash);
		}


		auto it = m_render_targets.find(hash);
		if (it != m_render_targets.end())
		{
			return it->second;
		}
		OpenGL_RenderTarget* rt = new OpenGL_RenderTarget();
		rt->m_index             = hash;
		m_render_targets[hash]  = rt;

		rt->init(color_attachments, depth_stencil);
		return rt;
	}

	void OpenGL_RenderTarget::bind(bool override)
	{
		if (override)
			OPENGL_API->m_state.render_target = this;
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	}

	template<typename T>
	static GLuint create_framebuffer(OpenGL_RenderTarget* self, const Span<T*>& color_attachments, T* depth_stencil,
	                                 OpenGL_RenderSurface* (*callback)(T*) )
	{
		GLuint m_framebuffer = 0;
		glGenFramebuffers(1, &m_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

		size_t index = 0;

		Vector<GLenum> color_attachment_indices;
		color_attachment_indices.reserve(color_attachments.size());

		for (const auto& color_attachment : color_attachments)
		{
			auto color_texture = color_attachment;
			info_log("Framebuffer", "Attaching texture[%p] to buffer %p", color_texture, self);
			self->attach_texture(callback(color_texture), GL_COLOR_ATTACHMENT0 + index);
			color_attachment_indices.push_back(GL_COLOR_ATTACHMENT0 + index);
			index++;
		}

		if (depth_stencil)
		{
			self->attach_texture(callback(depth_stencil), get_attachment_type(callback(depth_stencil)->m_format.m_format));
		}

		if (color_attachment_indices.size() > 0)
		{
			glDrawBuffers(color_attachment_indices.size(), color_attachment_indices.data());
		}
		return m_framebuffer;
	}

	OpenGL_RenderTarget& OpenGL_RenderTarget::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
	{
		m_framebuffer = create_framebuffer<RenderSurface>(
		        this, color_attachments, depth_stencil,
		        [](RenderSurface* texture) -> OpenGL_RenderSurface* { return texture->rhi_object<OpenGL_RenderSurface>(); });
		return *this;
	}

	OpenGL_RenderTarget& OpenGL_RenderTarget::init(const Span<struct OpenGL_RenderSurface*>& color_attachments,
	                                               struct OpenGL_RenderSurface* depth_stencil)
	{
		m_framebuffer = create_framebuffer<OpenGL_RenderSurface>(
		        this, color_attachments, depth_stencil,
		        [](OpenGL_RenderSurface* texture) -> OpenGL_RenderSurface* { return texture; });
		return *this;
	}

	OpenGL_RenderTarget& OpenGL_RenderTarget::attach_texture(const OpenGL_RenderSurface* texture, GLuint attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture->m_type, texture->m_id, 0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		{
			error_log("Framebuffer", "Incomplete framebuffer attachments\n");
		}
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		{
			error_log("Framebuffer", "incomplete missing framebuffer attachments");
		}
#if USING_OPENGL_CORE
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
		{
			error_log("Framebuffer", "Incomplete framebuffer attachments dimensions\n");
		}
#endif
		else if (status == GL_FRAMEBUFFER_UNSUPPORTED)
		{
			error_log("Framebuffer", "Combination of internal formats used by attachments in thef ramebuffer results in a "
			                         "nonrednerable target");
		}
		else
		{
			info_log("Framebuffer", "Attach success!\n");
		}

		m_textures.push_back(texture);
		texture->m_render_targets.insert(this);
		return *this;
	}

	OpenGL_RenderTarget::~OpenGL_RenderTarget()
	{
		if (m_framebuffer != 0)
		{
			glDeleteFramebuffers(1, &m_framebuffer);
		}

		for (auto& surface : m_textures)
		{
			surface->m_render_targets.erase(this);
		}

		m_render_targets.erase(m_index);
	}


	OpenGL_RenderTarget::Saver::Saver() : m_viewport(OPENGL_API->viewport()), m_rt(OpenGL_RenderTarget::current())
	{}

	OpenGL_RenderTarget::Saver::~Saver()
	{
		if (m_rt)
		{
			m_rt->bind();
		}

		OPENGL_API->viewport(m_viewport);
	}

	OpenGL& OpenGL::bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
	{
		auto rt = OpenGL_RenderTarget::find_or_create(color_attachments, depth_stencil);
		rt->bind();
		return *this;
	}

	OpenGL_RenderTarget* OpenGL::bind_render_target(const Span<struct OpenGL_RenderSurface*>& color_attachments,
	                                                struct OpenGL_RenderSurface* depth_stencil)
	{
		auto rt = OpenGL_RenderTarget::find_or_create(color_attachments, depth_stencil);
		rt->bind();
		return rt;
	}

	OpenGL& OpenGL::viewport(const ViewPort& viewport)
	{
		if (viewport != m_state.viewport)
		{
			glViewport(viewport.pos.x, viewport.pos.y, viewport.size.x, viewport.size.y);
			glDepthRangef(viewport.min_depth, viewport.max_depth);
			m_state.viewport = viewport;
		}

		return *this;
	}

	ViewPort OpenGL::viewport()
	{
		return m_state.viewport;
	}

	OpenGL& OpenGL::scissor(const Scissor& scissor)
	{
		if (scissor != m_state.scissor)
		{
			glScissor(scissor.pos.x, scissor.pos.y, scissor.size.x, scissor.size.y);
			m_state.scissor = scissor;
		}
		return *this;
	}

	Scissor OpenGL::scissor()
	{
		return m_state.scissor;
	}
}// namespace Engine
