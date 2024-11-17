#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/pointer.hpp>
#include <Event/event.hpp>
#include <Event/listener_id.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/properties_window.hpp>

namespace Engine
{
	class StaticMesh;
	class CameraComponent;
	class World;
	class StaticMeshActor;

	class StaticMeshClient : public ImGuiEditorClient
	{
		declare_class(StaticMeshClient, ImGuiEditorClient);

		Vector3D m_camera_move;
		Vector<EventSystemListenerID> m_listeners;
		Pointer<StaticMesh> m_mesh;
		StaticMeshActor* m_actor = nullptr;
		Pointer<CameraComponent> m_camera;
		Pointer<World> m_world;

		class ContentBrowser* m_browser = nullptr;
		ImGuiObjectProperties m_properties;
		SceneView m_view;
		EditorSceneRenderer m_renderer;

		StaticMeshClient& render_dock();
		StaticMeshClient& render_viewport(float dt);
		StaticMeshClient& render_properties();
		StaticMeshClient& update_camera(float dt);

	public:
		StaticMeshClient& on_bind_viewport(RenderViewport* vp) override;
		StaticMeshClient& on_unbind_viewport(RenderViewport* vp) override;
		StaticMeshClient& update(float dt) override;
		StaticMeshClient& select(Object* object) override;
		StaticMeshClient& render(RenderViewport* vp) override;
	};
}// namespace Engine
