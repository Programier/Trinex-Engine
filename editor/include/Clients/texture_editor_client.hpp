#pragma once
#include <Clients/imgui_client.hpp>

namespace Engine
{
	class Texture2D;
	class RenderSurface;
	class ImGuiObjectProperties;

	class TextureEditorClient : public ImGuiEditorClient
	{
		declare_class(TextureEditorClient, ImGuiEditorClient);

		Pointer<Texture2D> m_texture;
		Pointer<RenderSurface> m_surface;
		ImGuiObjectProperties* m_properties = nullptr;

		uint_t m_mip_index = 0;

		union
		{
			struct {
				bool red   = true;
				bool green = true;
				bool blue  = true;
				bool alpha = true;
			};

			bool channels_status[4];
		};

		TextureEditorClient& render_menu_bar();
		TextureEditorClient& render_dock();
		TextureEditorClient& render_texture();

		TextureEditorClient& on_object_parameters_changed(bool reinit = false);

	public:
		TextureEditorClient();
		TextureEditorClient& on_bind_viewport(RenderViewport* vp) override;
		TextureEditorClient& update(float dt) override;
		TextureEditorClient& select(Object* object) override;
	};
}// namespace Engine
