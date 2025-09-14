#pragma once
#include <Core/pointer.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
	class CameraComponent;
	class StaticMeshActor;
	class World;
	class MaterialInterface;

	class ImGuiStaticMeshPreview : public ImGuiWidget
	{
		Pointer<CameraComponent> m_camera;
		Pointer<StaticMeshActor> m_mesh;

		float m_target_zoom  = 1.f;
		float m_current_zoom = 1.f;

		Vector3f m_target_location  = {0, 0.707107, 0.707107};
		Vector3f m_current_location = {0, 0.707107, 0.707107};

		float m_lerp_speed        = 7.f;
		float m_mouse_sensitivity = 1.f;

		int_t match_zoom_index(int direction);
		float match_zoom(int steps, float fallback);

	public:
		ImGuiStaticMeshPreview();
		RenderSurface* render_preview(ImVec2 size);
		ImGuiStaticMeshPreview& update_zoom();
		ImGuiStaticMeshPreview& update_rotation(const ImVec2& size);
		ImGuiStaticMeshPreview& update_input(const ImVec2& size);
		ImGuiStaticMeshPreview& mesh(StaticMeshActor* actor);
		bool render(RenderViewport* viewport = nullptr) override;

		virtual const char* name();
		static const char* static_name();
	};
}// namespace Engine
