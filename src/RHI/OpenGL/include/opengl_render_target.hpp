#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
	struct OpenGL_Surface;
	struct OpenGL_SurfaceRTV;
	struct OpenGL_SurfaceDSV;

	struct OpenGL_RenderTarget {
		static TreeMap<HashIndex, OpenGL_RenderTarget*> m_render_targets;

		OpenGL_SurfaceRTV* m_RTVs[4] = {nullptr, nullptr, nullptr, nullptr};
		OpenGL_SurfaceDSV* m_DSV     = nullptr;
		GLuint m_framebuffer         = 0;
		HashIndex m_index            = 0;

		static void release_all();
		static OpenGL_RenderTarget* current();
		static OpenGL_RenderTarget* find_or_create(OpenGL_SurfaceRTV* rt1           = nullptr,//
												   OpenGL_SurfaceRTV* rt2           = nullptr,//
												   OpenGL_SurfaceRTV* rt3           = nullptr,//
												   OpenGL_SurfaceRTV* rt4           = nullptr,//
												   OpenGL_SurfaceDSV* depth_stencil = nullptr);

		static inline OpenGL_RenderTarget* find_or_create(OpenGL_SurfaceDSV* depth_stencil)
		{
			return find_or_create(nullptr, nullptr, nullptr, nullptr, depth_stencil);
		}

		void bind(bool override = true);
		OpenGL_RenderTarget& init(OpenGL_SurfaceRTV** targets, struct OpenGL_SurfaceDSV* depth_stencil);
		OpenGL_RenderTarget& attach_texture(OpenGL_Surface* surface, GLuint attachment);

		~OpenGL_RenderTarget();

		struct Saver {
			ViewPort m_viewport;
			OpenGL_RenderTarget* m_rt;

			Saver();
			~Saver();
		};
	};


}// namespace Engine
