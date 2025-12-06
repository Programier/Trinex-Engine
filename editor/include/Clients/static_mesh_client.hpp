#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/pointer.hpp>
#include <Engine/scene_view.hpp>
#include <Widgets/imgui_windows.hpp>

namespace Engine
{
	class StaticMesh;
	class CameraComponent;
	class World;
	class StaticMeshActor;
	class RenderSurface;
	class PropertyRenderer;

	class StaticMeshClient : public ImGuiViewportClient
	{
		trinex_class(StaticMeshClient, ImGuiViewportClient);

	private:
		class Viewport;

		Vector3f m_camera_move;
		Vector<Identifier> m_listeners;
		Pointer<StaticMesh> m_mesh;
		StaticMeshActor* m_actor = nullptr;
		Pointer<CameraComponent> m_camera;
		Pointer<World> m_world;
		Viewport* m_viewport = nullptr;

		class ContentBrowser* m_browser = nullptr;
		PropertyRenderer* m_property_renderer;
		SceneView m_view;

	public:
		StaticMeshClient();
		~StaticMeshClient();

		StaticMeshClient& on_bind_viewport(RenderViewport* vp) override;
		StaticMeshClient& on_unbind_viewport(RenderViewport* vp) override;
		StaticMeshClient& update(float dt) override;
		StaticMeshClient& select(Object* object) override;
		uint32_t build_dock(uint32_t dock) override;
	};
}// namespace Engine
