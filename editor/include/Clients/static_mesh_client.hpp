#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/pointer.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/property_renderer.hpp>

namespace Engine
{
	class StaticMesh;
	class CameraComponent;
	class World;
	class StaticMeshActor;

	class StaticMeshClient : public ImGuiViewportClient
	{
		trinex_declare_class(StaticMeshClient, ImGuiViewportClient);

		Vector3f m_camera_move;
		Vector<Identifier> m_listeners;
		Pointer<StaticMesh> m_mesh;
		StaticMeshActor* m_actor = nullptr;
		Pointer<CameraComponent> m_camera;
		Pointer<World> m_world;

		class ContentBrowser* m_browser = nullptr;
		PropertyRenderer m_property_renderer;
		SceneView m_view;
		Renderer<EditorSceneRenderer> m_renderer;

		StaticMeshClient& render_viewport(float dt);
		StaticMeshClient& render_properties();
		StaticMeshClient& update_camera(float dt);

	public:
		StaticMeshClient& on_bind_viewport(RenderViewport* vp) override;
		StaticMeshClient& on_unbind_viewport(RenderViewport* vp) override;
		StaticMeshClient& update(float dt) override;
		StaticMeshClient& select(Object* object) override;
		StaticMeshClient& render(RenderViewport* vp) override;
		uint32_t build_dock(uint32_t dock) override;
	};
}// namespace Engine
