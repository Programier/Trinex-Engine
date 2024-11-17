#include <Clients/static_mesh_client.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>
#include <Engine/world.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Widgets/content_browser.hpp>
#include <Window/window.hpp>
#include <imgui_internal.h>

namespace Engine
{
	implement_engine_class(StaticMeshClient, 0)
	{
		register_client(StaticMesh::static_class_instance(), static_class_instance());
	}

	StaticMeshClient& StaticMeshClient::on_bind_viewport(RenderViewport* vp)
	{
		Super::on_bind_viewport(vp);

		m_world = System::new_system<World>();

		m_camera = new_instance<CameraComponent>();
		m_camera->location({0, 3, 5});
		m_camera->rotation({-30, 0, 0});
		m_view.camera_view(m_camera->camera_view());
		m_renderer.scene = m_world->scene();

		m_browser           = imgui_window()->widgets_list.create<ContentBrowser>();
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

	StaticMeshClient& StaticMeshClient::render_dock()
	{
		auto dock_id                       = ImGui::GetID("StaticMeshClient##Dock");
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags | ImGuiDockNodeFlags_NoTabBar);

		if (imgui_window()->frame_index() == 1)
		{

			ImGui::DockBuilderRemoveNode(dock_id);
			ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);

			auto dock_id_right  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.35f, nullptr, &dock_id);
			auto dock_id_botton = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.35f, nullptr, &dock_id);

			ImGui::DockBuilderDockWindow("###Viewport", dock_id);
			ImGui::DockBuilderDockWindow(m_properties.name(), dock_id_right);
			ImGui::DockBuilderDockWindow(ContentBrowser::static_name(), dock_id_botton);
			ImGui::DockBuilderFinish(dock_id);
		}
		return *this;
	}

	StaticMeshClient& StaticMeshClient::render_viewport(float dt)
	{
		if (!ImGui::Begin("Viewport###Viewport", nullptr))
		{
			ImGui::End();
			return *this;
		};

		auto size              = ImGui::GetContentRegionAvail();
		m_camera->aspect_ratio = size.x / size.y;
		auto k                 = viewport()->size() / SceneRenderTargets::instance()->size();
		ImGui::Image(reinterpret_cast<Texture2D*>(m_renderer.output_surface()), size, {0.f, k.y}, {k.x, 0.f});

		ImGui::End();
		return *this;
	}

	StaticMeshClient& StaticMeshClient::render_properties()
	{
		m_properties.render(viewport());
		return *this;
	}

	StaticMeshClient& StaticMeshClient::update(float dt)
	{
		Super::update(dt);
		ImGuiViewport* imgui_viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(imgui_viewport->WorkPos);
		ImGui::SetNextWindowSize(imgui_viewport->WorkSize);
		ImGui::Begin("StaticMeshClient", nullptr,
					 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
							 ImGuiWindowFlags_MenuBar);
		render_dock();
		render_viewport(dt);
		render_properties();
		update_camera(dt);

		ImGui::End();

		return *this;
	}

	StaticMeshClient& StaticMeshClient::select(Object* object)
	{
		if (auto mesh = instance_cast<StaticMesh>(object))
		{
			m_mesh = mesh;
			m_properties.update(object);

			if (m_actor)
			{
				m_world->destroy_actor(m_actor);
			}

			m_actor = instance_cast<StaticMeshActor>(m_world->spawn_actor(StaticMeshActor::static_class_instance()));
			m_actor->mesh_component()->mesh = mesh;
		}

		return *this;
	}

	static FORCE_INLINE void move_camera(Vector3D& move, Window* window)
	{
		move = {0, 0, 0};

		if (!MouseSystem::instance()->is_pressed(Mouse::Button::Right, window))
			return;

		KeyboardSystem* keyboard = KeyboardSystem::instance();

		move.z += keyboard->is_pressed(Keyboard::W) ? -10.f : 0.f;
		move.x += keyboard->is_pressed(Keyboard::A) ? -10.f : 0.f;
		move.z += keyboard->is_pressed(Keyboard::S) ? 10.f : 0.f;
		move.x += keyboard->is_pressed(Keyboard::D) ? 10.f : 0.f;
	}


	StaticMeshClient& StaticMeshClient::update_camera(float dt)
	{
		move_camera(m_camera_move, window());
		m_camera->add_location(Vector3D((m_camera->world_transform().rotation_matrix() * Vector4D(m_camera_move, 1.0))) * dt);
		call_in_render_thread([cam_view = m_camera->camera_view(), this]() { m_view.camera_view(cam_view); });
		return *this;
	}

	StaticMeshClient& StaticMeshClient::render(RenderViewport* vp)
	{
		m_view.viewport(vp->viewport_info());
		m_view.scissor(vp->scissor_info());
		m_renderer.render(m_view, vp);

		Super::render(vp);
		return *this;
	}

}// namespace Engine
