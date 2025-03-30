#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/pointer.hpp>

namespace Engine
{
	class PropertyRenderer;

	class ObjectViewClient : public ImGuiViewportClient
	{
		trinex_declare_class(ObjectViewClient, ImGuiViewportClient);

	private:
		PropertyRenderer* m_property_renderer = nullptr;
		Pointer<Object> m_object;

	public:
		ObjectViewClient();
		ObjectViewClient& create_properties_window();
		ObjectViewClient& on_bind_viewport(RenderViewport* vp) override;
		uint32_t build_dock(uint32_t dock) override;
		ObjectViewClient& update(float dt) override;
		ObjectViewClient& select(Object* object) override;

		inline Object* selected_object() const { return m_object.ptr(); }
	};
}// namespace Engine
