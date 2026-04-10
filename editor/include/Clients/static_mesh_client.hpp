#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/pointer.hpp>
#include <Engine/Render/scene_view.hpp>
#include <Widgets/imgui_windows.hpp>

namespace Trinex
{
	class StaticMesh;
	class CameraComponent;
	class World;
	class StaticMeshActor;
	class SkeletalMeshActor;
	class RenderSurface;
	class PropertyRenderer;

	class StaticMeshClient : public ImGuiViewportClient
	{
		trinex_class(StaticMeshClient, ImGuiViewportClient);

	private:
		class Viewport;

		Vector3f m_camera_move;
		Vector<Identifier> m_listeners;
		Pointer<CameraComponent> m_camera;
		Pointer<World> m_world;
		Viewport* m_viewport = nullptr;

		StaticMeshActor* m_static_mesh = nullptr;

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
		u32 build_dock(u32 dock) override;
	};
}// namespace Trinex
