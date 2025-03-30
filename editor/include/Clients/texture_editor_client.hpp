#pragma once
#include <Clients/imgui_client.hpp>

namespace Engine
{
	class Texture2D;
	class RenderSurface;
	class PropertyRenderer;

	class TextureEditorClient : public ImGuiViewportClient
	{
		trinex_declare_class(TextureEditorClient, ImGuiViewportClient);

		Pointer<RenderSurface> m_surface;
		PropertyRenderer* m_properties   = nullptr;
		size_t m_last_render_frame_index = 0;

		Swizzle m_swizzle;

		float m_pow = 1.f;

	protected:
		virtual TextureEditorClient& setup_surface(RenderSurface* dst)      = 0;
		virtual TextureEditorClient& rhi_render_surface(RenderSurface* dst) = 0;
		TextureEditorClient& request_render();

	public:
		TextureEditorClient();
		TextureEditorClient& on_bind_viewport(RenderViewport* vp) override;
		TextureEditorClient& update(float dt) override;
		TextureEditorClient& select(Object* object) override;
		uint32_t build_dock(uint32_t dock_id) override;

		inline Swizzle swizzle() const { return m_swizzle; }
		inline float pow_factor() const { return m_pow; }
	};

	class Texture2DEditorClient : public TextureEditorClient
	{
		trinex_declare_class(Texture2DEditorClient, TextureEditorClient);

	private:
		uint32_t m_mip_index = 0;
		Pointer<Texture2D> m_texture;

	protected:
		Texture2DEditorClient& setup_surface(RenderSurface* dst) override;
		Texture2DEditorClient& rhi_render_surface(RenderSurface* dst) override;

	public:
		Texture2DEditorClient& select(Object* object) override;
	};

	class RenderSurfaceEditorClient : public TextureEditorClient
	{
		trinex_declare_class(RenderSurfaceEditorClient, TextureEditorClient);

	private:
		Pointer<RenderSurface> m_surface;
		bool m_live_update = true;

	protected:
		RenderSurfaceEditorClient& setup_surface(RenderSurface* dst) override;
		RenderSurfaceEditorClient& rhi_render_surface(RenderSurface* dst) override;

	public:
		RenderSurfaceEditorClient();
		RenderSurfaceEditorClient& select(Object* object) override;
		RenderSurfaceEditorClient& update(float dt) override;
	};
}// namespace Engine
