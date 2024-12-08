#include <Clients/imgui_client.hpp>
#include <Core/etl/templates.hpp>
#include <Core/localization.hpp>
#include <Core/reflection/class.hpp>
#include <Core/theme.hpp>
#include <Graphics/imgui.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	static ScriptFunction m_ic_script_update;
	static ScriptFunction m_ic_script_select;

	implement_engine_class(ImGuiEditorClient, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_class_instance());

		m_ic_script_update = r.method("void update(float dt)", trinex_scoped_method(This, update, ImGuiEditorClient&, float));
		m_ic_script_select = r.method("void select(Object@ object)", trinex_scoped_method(This, select));

		r.method("void update(RenderViewport viewport, float dt)",
				 trinex_scoped_method(This, update, ImGuiEditorClient&, RenderViewport*, float));
		r.method("void on_bind_viewport(RenderViewport)", trinex_scoped_method(This, on_bind_viewport));
		r.method("void on_unbind_viewport(RenderViewport)", trinex_scoped_method(This, on_unbind_viewport));
		r.method("RenderViewport viewport() const final", &This::viewport);

		ScriptEngine::on_terminate += []() {
			m_ic_script_update.release();
			m_ic_script_select.release();
		};
	}

	static Map<Refl::Class*, ImGuiEditorClient*> m_opened_clients;
	static Map<Refl::Class*, Refl::Class*> m_viewports_map;

	static ImGuiEditorClient* open_editor_client(Refl::Class* client)
	{
		WindowConfig new_config;
		new_config.client = client->full_name();
		auto window       = WindowManager::instance()->create_window(new_config);
		return Object::instance_cast<ImGuiEditorClient>(window->render_viewport()->client());
	}

	static void draw_available_clients_for_opening_internal(Refl::Class* self, Refl::Class* skip)
	{
		if (self == nullptr)
			return;

		if (self != skip && self != ImGuiEditorClient::static_class_instance())
		{
			String fmt = Localization::instance()->localize(Strings::format("editor/Open {}", self->display_name()));

			if (ImGui::MenuItem(fmt.c_str(), nullptr, false, !m_opened_clients.contains(self)))
			{
				open_editor_client(self);
			}
		}

		for (Refl::Struct* child : self->childs())
		{
			if (auto class_instance = Refl::Object::instance_cast<Refl::Class>(child))
			{
				draw_available_clients_for_opening_internal(class_instance, skip);
			}
		}
	}

	void ImGuiEditorClient::scriptable_update(float dt)
	{
		ScriptObject obj(this);
		obj.execute(m_ic_script_update, dt);
	}

	void ImGuiEditorClient::scriptable_select(Object* object)
	{
		ScriptObject obj(this);
		obj.execute(m_ic_script_update, object);
	}

	bool ImGuiEditorClient::register_client(Refl::Class* object_type, Refl::Class* renderer)
	{
		if (object_type == nullptr || renderer == nullptr)
			return false;

		if (!renderer->is_a(static_class_instance()) || object_type->is_a(renderer) || renderer->is_a(object_type))
			return false;

		m_viewports_map.insert({object_type, renderer});
		return true;
	}

	ImGuiEditorClient* ImGuiEditorClient::client_of(Refl::Class* object_type, bool create_if_not_exist)
	{
		Refl::Class* viewport_class = nullptr;

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

		if (window == nullptr)
			throw EngineException("ImGuiEditorClient requires valid window object!");

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
