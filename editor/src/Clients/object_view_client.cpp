#include <Clients/object_view_client.hpp>
#include <Core/reflection/class.hpp>
#include <Widgets/property_renderer.hpp>
#include <imgui_internal.h>

namespace Engine
{
	implement_engine_class(ObjectViewClient, 0)
	{
		register_client(Object::static_class_instance(), static_class_instance());
	}

	ObjectViewClient& ObjectViewClient::create_properties_window()
	{
		if (m_property_renderer == nullptr)
		{
			m_property_renderer = imgui_window()->widgets_list.create<PropertyRenderer>();
			m_property_renderer->on_close.push([this]() { m_property_renderer = nullptr; });
		}
		return *this;
	}

	ObjectViewClient& ObjectViewClient::on_bind_viewport(RenderViewport* vp)
	{
		Super::on_bind_viewport(vp);
		create_properties_window();
		return *this;
	}

	ObjectViewClient& ObjectViewClient::build_dock(uint32_t dock_id)
	{
		ImGui::DockBuilderDockWindow(PropertyRenderer::static_name(), dock_id);
		return *this;
	}

	ObjectViewClient& ObjectViewClient::update(float dt)
	{
		Super::update(dt);
		return *this;
	}

	ObjectViewClient& ObjectViewClient::select(Object* object)
	{
		Super::select(object);

		if (m_property_renderer)
		{
			m_property_renderer->update(object);
		}
		return *this;
	}

	ObjectViewClient& ObjectViewClient::render_menu_bar()
	{
		return *this;
	}
}// namespace Engine
