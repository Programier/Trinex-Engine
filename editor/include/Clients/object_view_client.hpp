#pragma once
#include <Clients/imgui_client.hpp>


namespace Engine
{
	class PropertyRenderer;

	class ObjectViewClient : public ImGuiViewportClient
	{
		declare_class(ObjectViewClient, ImGuiViewportClient);

	private:
		PropertyRenderer* m_property_renderer = nullptr;

	private:
		ObjectViewClient& create_properties_window();
		ObjectViewClient& on_bind_viewport(RenderViewport* vp) override;
		ObjectViewClient& build_dock(uint32_t dock_id) override;
		ObjectViewClient& update(float dt) override;
		ObjectViewClient& select(Object* object) override;

		virtual ObjectViewClient& render_menu_bar();
	};
}// namespace Engine
