#include <Clients/static_mesh_client.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/world.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/mesh_preview.hpp>
#include <Widgets/property_renderer.hpp>
#include <Window/window.hpp>
#include <imgui_internal.h>

namespace Engine
{
	trinex_implement_engine_class(StaticMeshClient, 0)
	{
		register_client(StaticMesh::static_reflection(), static_reflection());
	}

	class StaticMeshClient::Viewport : public ImGuiStaticMeshPreview
	{
	public:
		static const char* static_name() { return "editor/Viewport"_localized; }
		const char* name() override { return static_name(); }
	};

	StaticMeshClient::StaticMeshClient()
	{
		m_viewport          = trx_new Viewport();
		m_property_renderer = trx_new PropertyRenderer();
	}

	StaticMeshClient::~StaticMeshClient()
	{
		trx_delete m_property_renderer;
		trx_delete m_viewport;
	}

	StaticMeshClient& StaticMeshClient::on_bind_viewport(RenderViewport* vp)
	{
		Super::on_bind_viewport(vp);

		m_world = System::system_of<World>();

		m_camera = new_instance<CameraComponent>();
		m_camera->location({0, 3, 5});
		m_camera->rotation({-30, 0, 0});
		m_view.camera_view(m_camera->camera_view());

		m_browser           = window()->widgets.create<ContentBrowser>();
		m_browser->closable = false;
		return *this;
	}

	StaticMeshClient& StaticMeshClient::on_unbind_viewport(RenderViewport* vp)
	{
		Super::on_unbind_viewport(vp);

		m_browser = nullptr;
		for (auto& listener : m_listeners)
		{
			EventSystem::instance()->remove_listener(listener);
		}

		return *this;
	}

	StaticMeshClient& StaticMeshClient::update(float dt)
	{
		Super::update(dt);
		m_viewport->render(viewport());
		m_property_renderer->render(viewport());
		return *this;
	}

	StaticMeshClient& StaticMeshClient::select(Object* object)
	{
		if (auto mesh = instance_cast<StaticMesh>(object))
		{
			m_mesh = mesh;
			m_property_renderer->object(object);

			if (m_actor)
			{
				m_world->destroy_actor(m_actor);
			}

			m_actor = instance_cast<StaticMeshActor>(m_world->spawn_actor(StaticMeshActor::static_reflection()));
			m_actor->mesh_component()->mesh(mesh);
			m_viewport->mesh(m_actor);
		}

		return *this;
	}

	uint32_t StaticMeshClient::build_dock(uint32_t dock)
	{
		auto dock_right  = ImGui::DockBuilderSplitNode(dock, ImGuiDir_Right, 0.35f, nullptr, &dock);
		auto dock_botton = ImGui::DockBuilderSplitNode(dock, ImGuiDir_Down, 0.35f, nullptr, &dock);

		ImGui::DockBuilderDockWindow(m_viewport->name(), dock);
		ImGui::DockBuilderDockWindow(m_property_renderer->name(), dock_right);
		ImGui::DockBuilderDockWindow(ContentBrowser::static_name(), dock_botton);
		return dock;
	}

}// namespace Engine
