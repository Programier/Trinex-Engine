#include <Clients/object_view_client.hpp>
#include <Core/reflection/class.hpp>
#include <Widgets/property_renderer.hpp>
#include <imgui_internal.h>

namespace Engine
{
	trinex_implement_engine_class(ObjectViewClient, 0)
	{
		register_client(Object::static_class_instance(), static_class_instance());
	}

	ObjectViewClient::ObjectViewClient()
	{
		menu_bar.create("editor/View")->actions.push([this]() {
			draw_available_clients_for_opening();

			if (ImGui::MenuItem(PropertyRenderer::static_name(), nullptr, false, m_property_renderer == nullptr))
			{
				create_properties_window();
			}
		});
	}

	ObjectViewClient& ObjectViewClient::create_properties_window()
	{
		if (m_property_renderer == nullptr)
		{
			m_property_renderer = imgui_window()->widgets_list.create<PropertyRenderer>();
			m_property_renderer->on_close.push([this]() { m_property_renderer = nullptr; });

			if (m_object)
			{
				m_property_renderer->update(m_object);
			}
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
		m_object = object;

		if (m_property_renderer)
		{
			m_property_renderer->update(object);
		}
		return *this;
	}
}// namespace Engine
