#include <Clients/imgui_client.hpp>
#include <Core/class.hpp>
#include <Core/localization.hpp>
#include <Core/theme.hpp>
#include <Graphics/imgui.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>


namespace Engine
{
	implement_engine_class(ImGuiEditorClient, Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = [](ScriptClassRegistrar* r, Class*) {
			r->method("void update(float dt)", method_of<ImGuiEditorClient&, float>(&This::update));
			r->method("void on_bind_viewport(RenderViewport)", &This::on_bind_viewport);
			r->method("void on_unbind_viewport(RenderViewport)", &This::on_unbind_viewport);
			r->method("RenderViewport viewport() const final", &This::viewport);
		};
	}

	static Map<Class*, ImGuiEditorClient*> m_opened_clients;
	static Map<Class*, Class*> m_viewports_map;

	static ImGuiEditorClient* open_editor_client(Class* client)
	{
		WindowConfig new_config;
		new_config.client = client->name().to_string();
		auto window       = WindowManager::instance()->create_window(new_config);
		return Object::instance_cast<ImGuiEditorClient>(window->render_viewport()->client());
	}

	static void draw_available_clients_for_opening_internal(Class* self, Class* skip)
	{
		if (self == nullptr)
			return;

		if (self != skip && self != ImGuiEditorClient::static_class_instance())
		{
			String fmt = Localization::instance()->localize(Strings::format("editor/Open {}", self->base_name_splitted()));

			if (ImGui::MenuItem(fmt.c_str(), nullptr, false, !m_opened_clients.contains(self)))
			{
				open_editor_client(self);
			}
		}

		for (Class* child : self->child_classes())
		{
			draw_available_clients_for_opening_internal(child, skip);
		}
	}

	bool ImGuiEditorClient::register_client(Class* object_type, Class* renderer)
	{
		if (object_type == nullptr || renderer == nullptr)
			return false;

		if (!renderer->is_a(static_class_instance()) || object_type->is_a(renderer) || renderer->is_a(object_type))
			return false;

		m_viewports_map.insert({object_type, renderer});
		return true;
	}

	ImGuiEditorClient* ImGuiEditorClient::client_of(Class* object_type, bool create_if_not_exist)
	{
		Class* viewport_class = nullptr;

		while (object_type && viewport_class == nullptr)
		{
			auto it = m_viewports_map.find(object_type);

			if (it != m_viewports_map.end())
			{
				viewport_class = it->second;
				break;
			}

			object_type = object_type->parent();
		}

		if (viewport_class)
		{
			auto it = m_opened_clients.find(viewport_class);

			if (it != m_opened_clients.end())
				return it->second;

			if (create_if_not_exist)
			{
				return open_editor_client(viewport_class);
			}
		}

		return nullptr;
	}

	void ImGuiEditorClient::draw_available_clients_for_opening()
	{
		draw_available_clients_for_opening_internal(This::static_class_instance(), class_instance());
	}

	ImGuiEditorClient& ImGuiEditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		m_opened_clients.insert({class_instance(), this});
		Super::on_bind_viewport(viewport);
		auto window = viewport->window();

		m_window = Object::new_instance<ImGuiWindow>();
		m_window->initialize(window, EditorTheme::initialize_theme);
		m_viewport = viewport;

		return *this;
	}

	ImGuiEditorClient& ImGuiEditorClient::on_unbind_viewport(class RenderViewport* viewport)
	{
		m_opened_clients.erase(class_instance());

		Super::on_unbind_viewport(viewport);
		m_window->terminate();
		m_window   = nullptr;
		m_viewport = nullptr;

		return *this;
	}

	ImGuiEditorClient& ImGuiEditorClient::render(class RenderViewport* viewport)
	{
		Super::render(viewport);
		viewport->rhi_bind();
		viewport->rhi_clear_color(Color(0, 0, 0, 1));
		m_window->rhi_render();
		return *this;
	}

	ImGuiEditorClient& ImGuiEditorClient::update(class RenderViewport* viewport, float dt)
	{
		Super::update(viewport, dt);

		m_window->new_frame();
		update(dt);
		m_window->end_frame();
		return *this;
	}

	ImGuiWindow* ImGuiEditorClient::imgui_window() const
	{
		return m_window.ptr();
	}

	Window* ImGuiEditorClient::window() const
	{
		return m_window ? m_window->window() : nullptr;
	}

	RenderViewport* ImGuiEditorClient::viewport() const
	{
		return m_viewport;
	}

	ImGuiEditorClient& ImGuiEditorClient::update(float dt)
	{
		return *this;
	}

	ImGuiEditorClient& ImGuiEditorClient::select(Object* object)
	{
		return *this;
	}
}// namespace Engine
