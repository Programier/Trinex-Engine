#pragma once
#include <Core/etl/set.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_color_format.hpp>
#include <opengl_headers.hpp>
#include <opengl_resource.hpp>

namespace Engine
{
	struct OpenGL_Texture : public RHI_DefaultDestroyable<RHI_Texture> {
		OpenGL_ColorInfo m_format;
		GLuint m_type = 0;
		GLuint m_id   = 0;
		Size2D m_size;

		void init(const Texture2D* texture);
		RHI_ShaderResourceView* create_srv() override;
		RHI_UnorderedAccessView* create_uav() override;
		~OpenGL_Texture();
	};

	struct OpenGL_TextureSRV : OpenGL_SRV {
		OpenGL_Texture* m_texture;

		OpenGL_TextureSRV(OpenGL_Texture* texture);
		void bind(BindLocation location) override;
		void bind_combined(byte location, struct RHI_Sampler* sampler) override;
		~OpenGL_TextureSRV();
	};

	struct OpenGL_TextureUAV : OpenGL_UAV {
		OpenGL_Texture* m_texture;

		OpenGL_TextureUAV(OpenGL_Texture* texture);
		void bind(BindLocation location) override;
		~OpenGL_TextureUAV();
	};
}// namespace Engine
