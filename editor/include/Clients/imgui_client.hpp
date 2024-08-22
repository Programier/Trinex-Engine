#pragma once
#include <Core/pointer.hpp>
#include <Graphics/render_viewport.hpp>


namespace Engine
{
	class ImGuiWindow;
	class Window;

	class ImGuiEditorClient : public ViewportClient
	{
		declare_class(ImGuiEditorClient, ViewportClient);

	private:
		Pointer<ImGuiWindow> m_window;
		RenderViewport* m_viewport = nullptr;

	public:
		ImGuiEditorClient& on_bind_viewport(class RenderViewport* viewport) override;
		ImGuiEditorClient& on_unbind_viewport(class RenderViewport* viewport) override;
		ImGuiEditorClient& update(class RenderViewport* viewport, float dt) override;
		ImGuiEditorClient& render(class RenderViewport* viewport) override;

		ImGuiWindow* imgui_window() const;
		const ImGuiEditorClient& imgui_new_frame() const;
		const ImGuiEditorClient& imgui_end_frame() const;
		Window* window() const;
		RenderViewport* viewport() const;
	};
}// namespace Engine
