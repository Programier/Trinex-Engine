#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/pointer.hpp>

namespace Trinex
{
	class PropertyRenderer;

	class ObjectViewClient : public ImGuiViewportClient
	{
		trinex_class(ObjectViewClient, ImGuiViewportClient);

	private:
		PropertyRenderer* m_property_renderer = nullptr;
		Pointer<Object> m_object;

	public:
		ObjectViewClient();
		ObjectViewClient& create_properties_window();
		ObjectViewClient& on_bind_viewport(RenderViewport* vp) override;
		u32 build_dock(u32 dock) override;
		ObjectViewClient& update(float dt) override;
		ObjectViewClient& select(Object* object) override;

		inline Object* selected_object() const { return m_object.ptr(); }
	};
}// namespace Trinex
