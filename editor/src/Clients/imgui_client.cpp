#include <Clients/imgui_client.hpp>
#include <Core/class.hpp>
#include <Core/localization.hpp>
#include <Core/theme.hpp>
#include <Graphics/imgui.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Window/config.hpp>
#include <Window/window_manager.hpp>


namespace Engine
{
	implement_engine_class(ImGuiEditorClient, Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = [](ScriptClassRegistrar* r, Class*) {
			r->method("void update(RenderViewport viewport, float dt)", &This::update);
			r->method("void on_bind_viewport(RenderViewport)", &This::on_bind_viewport);
			r->method("void on_unbind_viewport(RenderViewport)", &This::on_unbind_viewport);
			r->method("RenderViewport viewport() const final", &This::viewport);
			r->method("void imgui_new_frame() const final", &This::imgui_new_frame);
			r->method("void imgui_end_frame() const final", &This::imgui_end_frame);
		};
	}

	static Set<Class*> m_opened_clients;

	static void draw_available_clients_for_opening_internal(Class* self, Class* skip)
	{
		if (self == nullptr)
			return;

		if (self != skip && self != ImGuiEditorClient::static_class_instance())
		{
			String fmt = Localization::instance()->localize(Strings::format("editor/Open {}", self->base_name_splitted()));

			if (ImGui::MenuItem(fmt.c_str(), nullptr, false, !m_opened_clients.contains(self)))
			{
				WindowConfig new_config;
				new_config.client = self->name().to_string();
				new_config.attributes.insert(WindowAttribute::Resizable);
				WindowManager::instance()->create_window(new_config);
			}
		}

		for (Class* child : self->child_classes())
		{
			draw_available_clients_for_opening_internal(child, skip);
		}
	}

	void ImGuiEditorClient::draw_available_clients_for_opening()
	{
		draw_available_clients_for_opening_internal(This::static_class_instance(), class_instance());
	}

	ImGuiEditorClient& ImGuiEditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		m_opened_clients.insert(class_instance());
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
		return *this;
	}

	ImGuiWindow* ImGuiEditorClient::imgui_window() const
	{
		return m_window.ptr();
	}

	const ImGuiEditorClient& ImGuiEditorClient::imgui_new_frame() const
	{
		m_window->new_frame();
		return *this;
	}

	const ImGuiEditorClient& ImGuiEditorClient::imgui_end_frame() const
	{
		m_window->end_frame();
		return *this;
	}

	Window* ImGuiEditorClient::window() const
	{
		return m_window ? m_window->window() : nullptr;
	}

	RenderViewport* ImGuiEditorClient::viewport() const
	{
		return m_viewport;
	}
}// namespace Engine
