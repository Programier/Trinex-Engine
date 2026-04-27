#pragma once

struct ImGuiContext;
struct ImDrawData;

namespace Trinex
{
	class Window;
	class RHIContext;
	struct Event;

	namespace UI::Backend
	{
		RHIContext* rhi();
		void imgui_init(Window* window, ImGuiContext* context);
		void imgui_shutdown(Window* window, ImGuiContext* context);
		void imgui_new_frame(Window* window);
		void imgui_render(RHIContext* ctx, ImDrawData* data);

		void imgui_event_recieved(const Event& event);
		void imgui_disable_events();
		void imgui_enable_events();
	}// namespace UI::Backend
}// namespace Trinex
