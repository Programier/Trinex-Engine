#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
	struct OpenGL_RenderSurface;
	struct OpenGL_RenderTarget {
		static TreeMap<HashIndex, OpenGL_RenderTarget*> m_render_targets;

		Vector<const struct OpenGL_RenderSurface*> m_textures;
		GLuint m_framebuffer = 0;
		size_t m_last_usage  = 0;
		HashIndex m_index    = 0;

		static void release_all();
		static OpenGL_RenderTarget* current();
		static OpenGL_RenderTarget* find_or_create(const RenderSurface* rt1, const RenderSurface* rt2, const RenderSurface* rt3,
												   const RenderSurface* rt4, RenderSurface* depth_stencil);

		static OpenGL_RenderTarget* find_or_create(const OpenGL_RenderSurface* rt1,//
												   const OpenGL_RenderSurface* rt2,//
												   const OpenGL_RenderSurface* rt3,//
												   const OpenGL_RenderSurface* rt4,//
												   struct OpenGL_RenderSurface* depth_stencil);

		void bind(bool override = true);
		OpenGL_RenderTarget& init(const OpenGL_RenderSurface** targets, struct OpenGL_RenderSurface* depth_stencil);
		OpenGL_RenderTarget& attach_texture(const struct OpenGL_RenderSurface* texture_attachmend, GLuint attachment);

		~OpenGL_RenderTarget();

		struct Saver {
			ViewPort m_viewport;
			OpenGL_RenderTarget* m_rt;

			Saver();
			~Saver();
		};
	};


}// namespace Engine
