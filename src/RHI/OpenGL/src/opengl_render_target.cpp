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
			for (int i = 0; i < 4; ++i) rt.second->m_RTVs[i] = nullptr;
			rt.second->m_DSV = nullptr;
			delete rt.second;
		}

		m_render_targets.clear();
	}

	OpenGL_RenderTarget* OpenGL_RenderTarget::current()
	{
		return OPENGL_API->m_state.render_target;
	}

	OpenGL_RenderTarget* OpenGL_RenderTarget::find_or_create(OpenGL_TextureRTV* rt1, OpenGL_TextureRTV* rt2,
															 OpenGL_TextureRTV* rt3, OpenGL_TextureRTV* rt4,
															 OpenGL_TextureDSV* depth_stencil)
	{
		OpenGL_TextureRTV* targets[4] = {rt1, rt2, rt3, rt4};

		HashIndex hash = memory_hash_fast(targets, sizeof(targets));
		hash           = memory_hash_fast(&depth_stencil, sizeof(depth_stencil), hash);

		auto it = m_render_targets.find(hash);
		if (it != m_render_targets.end())
		{
			return it->second;
		}
		OpenGL_RenderTarget* rt = new OpenGL_RenderTarget();
		rt->m_index             = hash;
		m_render_targets[hash]  = rt;

		rt->init(targets, depth_stencil);
		return rt;
	}

	void OpenGL_RenderTarget::bind(bool override)
	{
		if (override)
			OPENGL_API->m_state.render_target = this;
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	}

	OpenGL_RenderTarget& OpenGL_RenderTarget::init(OpenGL_TextureRTV** color_targets, OpenGL_TextureDSV* depth_stencil)
	{
		glGenFramebuffers(1, &m_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

		size_t index = 0;

		GLenum color_attachment_indices[4];

		for (int i = 0; i < 4; ++i)
		{
			if (color_targets[i])
			{
				m_RTVs[i] = color_targets[i];
				color_targets[i]->m_render_targets.insert(this);
				attach_texture(color_targets[i]->m_texture, GL_COLOR_ATTACHMENT0 + i);
				color_attachment_indices[index] = GL_COLOR_ATTACHMENT0 + i;
				++index;
			}
		}

		if (depth_stencil)
		{
			m_DSV = depth_stencil;
			depth_stencil->m_render_targets.insert(this);
			attach_texture(depth_stencil->m_texture, get_attachment_type(depth_stencil->m_texture->m_format.m_format));
		}

		if (index > 0)
		{
			glDrawBuffers(index, color_attachment_indices);
		}
		return *this;
	}

	OpenGL_RenderTarget& OpenGL_RenderTarget::attach_texture(OpenGL_Texture* texture, GLuint attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->m_id, 0);

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
		return *this;
	}

	OpenGL_RenderTarget::~OpenGL_RenderTarget()
	{
		if (m_framebuffer != 0)
		{
			glDeleteFramebuffers(1, &m_framebuffer);
		}

		for (auto& surface : m_RTVs)
		{
			if (surface)
				surface->m_render_targets.erase(this);
		}

		if (m_DSV)
			m_DSV->m_render_targets.erase(this);

		m_render_targets.erase(m_index);
	}


	OpenGL_RenderTarget::Saver::Saver() : m_viewport(OPENGL_API->viewport()), m_rt(OpenGL_RenderTarget::current()) {}

	OpenGL_RenderTarget::Saver::~Saver()
	{
		if (m_rt)
		{
			m_rt->bind();
		}

		OPENGL_API->viewport(m_viewport);
	}

	OpenGL& OpenGL::bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
									   RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil)
	{
		auto rt = OpenGL_RenderTarget::find_or_create(static_cast<OpenGL_TextureRTV*>(rt1), static_cast<OpenGL_TextureRTV*>(rt2),
													  static_cast<OpenGL_TextureRTV*>(rt3), static_cast<OpenGL_TextureRTV*>(rt4),
													  static_cast<OpenGL_TextureDSV*>(depth_stencil));
		rt->bind();
		return *this;
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
