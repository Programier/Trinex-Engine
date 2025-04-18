#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
	struct OpenGL_Texture;
	struct OpenGL_TextureRTV;
	struct OpenGL_TextureDSV;

	struct OpenGL_RenderTarget {
		static TreeMap<HashIndex, OpenGL_RenderTarget*> m_render_targets;

		OpenGL_TextureRTV* m_RTVs[4] = {nullptr, nullptr, nullptr, nullptr};
		OpenGL_TextureDSV* m_DSV     = nullptr;
		GLuint m_framebuffer         = 0;
		HashIndex m_index            = 0;

		static void release_all();
		static OpenGL_RenderTarget* current();
		static OpenGL_RenderTarget* find_or_create(OpenGL_TextureRTV* rt1           = nullptr,//
		                                           OpenGL_TextureRTV* rt2           = nullptr,//
		                                           OpenGL_TextureRTV* rt3           = nullptr,//
		                                           OpenGL_TextureRTV* rt4           = nullptr,//
		                                           OpenGL_TextureDSV* depth_stencil = nullptr);

		static inline OpenGL_RenderTarget* find_or_create(OpenGL_TextureDSV* depth_stencil)
		{
			return find_or_create(nullptr, nullptr, nullptr, nullptr, depth_stencil);
		}

		void bind(bool override = true);
		OpenGL_RenderTarget& init(OpenGL_TextureRTV** targets, struct OpenGL_TextureDSV* depth_stencil);
		OpenGL_RenderTarget& attach_texture(OpenGL_Texture* surface, GLuint attachment);

		~OpenGL_RenderTarget();

		struct Saver {
			ViewPort m_viewport;
			OpenGL_RenderTarget* m_rt;

			Saver();
			~Saver();
		};
	};


}// namespace Engine
