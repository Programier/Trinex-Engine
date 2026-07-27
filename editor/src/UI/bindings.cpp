#include <UI/reflection.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace Trinex::UI::Refl
{
	namespace
	{
		static ImVec2 window_pos()
		{
			return ImGui::GetWindowPos();
		}

		static ImVec2 window_size()
		{
			return ImGui::GetWindowSize();
		}

		static f32 window_width()
		{
			return ImGui::GetWindowWidth();
		}

		static f32 window_height()
		{
			return ImGui::GetWindowHeight();
		}

		static ImVec2 content_size()
		{
			return ImGui::GetContentRegionAvail();
		}

		static ImVec2 content_min()
		{
			return ImGui::GetWindowContentRegionMin();
		}

		static ImVec2 content_max()
		{
			return ImGui::GetWindowContentRegionMax();
		}

		static ImVec2 cursor_pos()
		{
			return ImGui::GetCursorPos();
		}

		static ImVec2 cursor_screen_pos()
		{
			return ImGui::GetCursorScreenPos();
		}

		static ImVec2 item_min()
		{
			return ImGui::GetItemRectMin();
		}

		static ImVec2 item_max()
		{
			return ImGui::GetItemRectMax();
		}

		static ImVec2 item_size()
		{
			return ImGui::GetItemRectSize();
		}

		static f32 scroll_x()
		{
			return ImGui::GetScrollX();
		}

		static f32 scroll_y()
		{
			return ImGui::GetScrollY();
		}

		static f32 scroll_max_x()
		{
			return ImGui::GetScrollMaxX();
		}

		static f32 scroll_max_y()
		{
			return ImGui::GetScrollMaxY();
		}

		static ImVec2 mouse_pos()
		{
			return ImGui::GetMousePos();
		}

		static f32 font_size()
		{
			return ImGui::GetFontSize();
		}

		static f32 frame_height()
		{
			return ImGui::GetFrameHeight();
		}

		static f32 frame_height_with_spacing()
		{
			return ImGui::GetFrameHeightWithSpacing();
		}
	}// namespace

	trinex_on_pre_init()
	{
		auto refl = NativeType<ImGuiContext*>::instance();
		refl->bind("window_pos", window_pos, Property::Markup);
		refl->bind("window_size", window_size, Property::Markup);
		refl->bind("window_width", window_width, Property::Markup);
		refl->bind("window_height", window_height, Property::Markup);
		refl->bind("content_size", content_size, Property::Markup);
		refl->bind("content_min", content_min, Property::Markup);
		refl->bind("content_max", content_max, Property::Markup);
		refl->bind("cursor_pos", cursor_pos, Property::Markup);
		refl->bind("cursor_screen_pos", cursor_screen_pos, Property::Markup);
		refl->bind("item_min", item_min, Property::Markup);
		refl->bind("item_max", item_max, Property::Markup);
		refl->bind("item_size", item_size, Property::Markup);
		refl->bind("scroll_x", scroll_x, Property::Markup);
		refl->bind("scroll_y", scroll_y, Property::Markup);
		refl->bind("scroll_max_x", scroll_max_x, Property::Markup);
		refl->bind("scroll_max_y", scroll_max_y, Property::Markup);
		refl->bind("mouse_pos", mouse_pos, Property::Markup);
		refl->bind("font_size", font_size, Property::Markup);
		refl->bind("frame_height", frame_height, Property::Markup);
		refl->bind("frame_height_with_spacing", frame_height_with_spacing, Property::Markup);
	}
}// namespace Trinex::UI::Refl
