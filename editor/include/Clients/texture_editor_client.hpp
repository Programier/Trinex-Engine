#pragma once
#include <Clients/imgui_client.hpp>

namespace Engine
{
	class Texture2D;
	class RenderSurface;
	class PropertyRenderer;

	class TextureEditorClient : public ImGuiViewportClient
	{
		declare_class(TextureEditorClient, ImGuiViewportClient);

		Pointer<Texture2D> m_texture;
		Pointer<RenderSurface> m_surface;
		PropertyRenderer* m_properties = nullptr;
		uint_t m_mip_index             = 0;

		union
		{
			struct {
				bool m_red   = true;
				bool m_green = true;
				bool m_blue  = true;
				bool m_alpha = true;
			};

			bool m_channels_status[4];
		};

		float m_pow        = 1.f;
		bool m_live_update = false;

		TextureEditorClient& render_texture();
		TextureEditorClient& on_object_parameters_changed(bool reinit = false);

	public:
		TextureEditorClient();
		TextureEditorClient& on_bind_viewport(RenderViewport* vp) override;
		TextureEditorClient& update(float dt) override;
		TextureEditorClient& select(Object* object) override;
		TextureEditorClient& build_dock(uint32_t dock_id) override;
	};
}// namespace Engine
