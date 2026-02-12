#include <Clients/imgui_client.hpp>
#include <Core/etl/templates.hpp>
#include <Core/localization.hpp>
#include <Core/reflection/class.hpp>
#include <Core/theme.hpp>
#include <Core/threading.hpp>
#include <Core/types/color.hpp>
#include <Graphics/imgui.hpp>
#include <RHI/rhi.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <imgui_internal.h>

namespace Engine
{
	static ScriptFunction m_ic_script_update;
	static ScriptFunction m_ic_script_select;

	trinex_implement_engine_class(ImGuiViewportClient, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_reflection());

		m_ic_script_update =
		        r.method("void update(float dt)", trinex_scoped_method(This, update, ImGuiViewportClient & (float) ));
		m_ic_script_select = r.method("void select(Object@ object)", trinex_scoped_method(This, select));

		r.method("void update(RenderViewport viewport, float dt)",
		         trinex_scoped_method(This, update, ImGuiViewportClient & (RenderViewport*, float) ));
		r.method("void on_bind_viewport(RenderViewport)", trinex_scoped_method(This, on_bind_viewport));
		r.method("void on_unbind_viewport(RenderViewport)", trinex_scoped_method(This, on_unbind_viewport));
		r.method("RenderViewport viewport() const final", &This::viewport);

		ScriptEngine::on_terminate += []() {
			m_ic_script_update.release();
			m_ic_script_select.release();
		};
	}

	static Map<Refl::Class*, ImGuiViewportClient*> m_opened_clients;
	static Map<Refl::Class*, Refl::Class*> m_viewports_map;

	static ImGuiViewportClient* open_editor_client(Refl::Class* client)
	{
		WindowConfig new_config;
		new_config.client = client->full_name();
		auto window       = WindowManager::instance()->create_window(new_config);
		return Object::instance_cast<ImGuiViewportClient>(window->render_viewport()->client());
	}

	static void draw_available_clients_for_opening_internal(Refl::Class* self, Refl::Class* skip)
	{
		if (self == nullptr)
			return;

		if (self != skip && self != ImGuiViewportClient::static_reflection())
		{
			String fmt = Localization::instance()->localize(Strings::format("editor/Open {}", self->display_name()));

			if (ImGui::MenuItem(fmt.c_str(), nullptr, false, !m_opened_clients.contains(self)))
			{
				open_editor_client(self);
			}
		}

		for (Refl::Struct* child : self->derived_structs())
		{
			if (auto class_instance = Refl::Object::instance_cast<Refl::Class>(child))
			{
				draw_available_clients_for_opening_internal(class_instance, skip);
			}
		}
	}

	static ImGuiContext* imgui_create_context()
	{
		ImGuiContext* context = ImGui::CreateContext();

		ImGuiContextLock lock(context);

		EditorTheme::initialize_theme(context);

#if !PLATFORM_ANDROID
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		return context;
	}

	void ImGuiViewportClient::scriptable_update(float dt)
	{
		ScriptContext::execute(this, m_ic_script_update, nullptr, dt);
	}

	void ImGuiViewportClient::scriptable_select(Object* object)
	{
		ScriptContext::execute(this, m_ic_script_select);
	}

	bool ImGuiViewportClient::register_client(Refl::Class* object_type, Refl::Class* renderer)
	{
		if (object_type == nullptr || renderer == nullptr)
			return false;

		m_viewports_map.insert({object_type, renderer});
		return true;
	}

	ImGuiViewportClient* ImGuiViewportClient::client_of(Refl::Class* object_type, bool create_if_not_exist)
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

	void ImGuiViewportClient::draw_available_clients_for_opening()
	{
		if (ImGui::BeginMenu("editor/Clients"_localized))
		{
			draw_available_clients_for_opening_internal(This::static_reflection(), class_instance());
			ImGui::EndMenu();
		}
	}

	ImGuiViewportClient::ImGuiViewportClient() : m_context(imgui_create_context()) {}

	ImGuiViewportClient::~ImGuiViewportClient()
	{
		ImGui::DestroyContext(m_context);
	}

	ImGuiViewportClient& ImGuiViewportClient::on_bind_viewport(class RenderViewport* viewport)
	{
		m_viewport = instance_cast<WindowRenderViewport>(viewport);
		trinex_always_check(m_viewport, "Viewport is invalid");

		m_opened_clients.insert({class_instance(), this});
		Super::on_bind_viewport(viewport);
		auto window = m_viewport->window();

		if (window == nullptr)
			throw EngineException("ImGuiViewportClient requires valid window object!");

		m_window = Object::new_instance<ImGuiWindow>();
		m_window->initialize(window, m_context);
		return *this;
	}

	ImGuiViewportClient& ImGuiViewportClient::on_unbind_viewport(class RenderViewport* viewport)
	{
		m_opened_clients.erase(class_instance());

		Super::on_unbind_viewport(viewport);
		m_window->terminate();
		m_window   = nullptr;
		m_viewport = nullptr;

		return *this;
	}

	ImGuiViewportClient& ImGuiViewportClient::update(class RenderViewport* viewport, float dt)
	{
		Super::update(viewport, dt);

		m_window->new_frame();

		ImGuiViewport* imgui_viewport      = ImGui::GetMainViewport();
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGui::DockSpaceOverViewport(s_dock_id, imgui_viewport, dockspace_flags);

		if (ImGui::IsWindowAppearing())
		{
			ImGui::DockBuilderRemoveNode(s_dock_id);
			ImGui::DockBuilderAddNode(s_dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(s_dock_id, imgui_viewport->WorkSize);

			build_dock(s_dock_id);
			ImGui::DockBuilderFinish(s_dock_id);
		}

		if (!menu_bar.is_empty())
		{
			if (ImGui::BeginMainMenuBar())
			{
				menu_bar.render();
				ImGui::EndMainMenuBar();
			}
		}

		update(dt);

		m_window->end_frame();
		return *this;
	}


	ImGuiViewportClient& ImGuiViewportClient::update(float dt)
	{
		return *this;
	}

	ImGuiViewportClient& ImGuiViewportClient::select(Object* object)
	{
		return *this;
	}

	uint32_t ImGuiViewportClient::build_dock(uint32_t dock)
	{
		return dock;
	}
}// namespace Engine
