#pragma once
#include <Core/etl/set.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_color_format.hpp>
#include <opengl_headers.hpp>
#include <opengl_resource.hpp>

namespace Engine
{
	struct OpenGL_Texture : public RHI_DefaultDestroyable<RHI_Texture> {
		RHI_ShaderResourceView* m_srv  = nullptr;
		RHI_UnorderedAccessView* m_uav = nullptr;
		RHI_RenderTargetView* m_rtv    = nullptr;
		RHI_DepthStencilView* m_dsv    = nullptr;

		OpenGL_ColorInfo m_format;
		GLuint m_id     = 0;
		Vector2u m_size = {0, 0};

		OpenGL_Texture& init_views(TextureCreateFlags flags);
		void init_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags);
		void update_2d(byte mip, const Rect2D& rect, const byte* data, size_t data_size);
		virtual GLuint type() = 0;

		RHI_RenderTargetView* create_rtv();
		RHI_DepthStencilView* create_dsv();
		RHI_ShaderResourceView* create_srv();
		RHI_UnorderedAccessView* create_uav();

		inline RHI_RenderTargetView* as_rtv() override { return m_rtv; }
		inline RHI_DepthStencilView* as_dsv() override { return m_dsv; }
		inline RHI_ShaderResourceView* as_srv() override { return m_srv; }
		inline RHI_UnorderedAccessView* as_uav() override { return m_uav; }
		~OpenGL_Texture();
	};

	template<GLuint m_type>
	struct OpenGL_TypedTexture : OpenGL_Texture {
		using OpenGL_Texture::OpenGL_Texture;
		GLuint type() override { return m_type; }
	};

	struct OpenGL_TextureSRV : OpenGL_SRV {
		OpenGL_Texture* m_texture;

		OpenGL_TextureSRV(OpenGL_Texture* texture);
		void bind(byte location, OpenGL_Sampler* sampler) override;
		~OpenGL_TextureSRV();
	};

	struct OpenGL_TextureUAV : OpenGL_UAV {
		OpenGL_Texture* m_texture;

		OpenGL_TextureUAV(OpenGL_Texture* texture);
		void bind(byte location) override;
		~OpenGL_TextureUAV();
	};

	struct OpenGL_TextureRTV : OpenGL_RTV {
		mutable Set<struct OpenGL_RenderTarget*> m_render_targets;
		OpenGL_Texture* const m_texture;

		OpenGL_TextureRTV(OpenGL_Texture* texture);
		void clear(const LinearColor& color) override;
		void blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		~OpenGL_TextureRTV();
	};

	struct OpenGL_TextureDSV : public OpenGL_DSV {
		mutable Set<struct OpenGL_RenderTarget*> m_render_targets;
		OpenGL_Texture* const m_texture;

		OpenGL_TextureDSV(OpenGL_Texture* texture);
		void clear(float depth, byte stencil) override;
		void blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		~OpenGL_TextureDSV();
	};

	struct OpenGL_Texture2D : public OpenGL_TypedTexture<GL_TEXTURE_2D> {
		OpenGL_Texture2D(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags);
	};

}// namespace Engine
