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

		template<typename Field, typename FlagsType>
		struct FlagsBinder {
			template<FlagsType flag>
			static bool load(const FlagsType* flags)
			{
				return (*flags) & flag;
			}

			template<FlagsType flag>
			static bool store(FlagsType* flags, bool status)
			{
				Field value = *flags;

				if (status)
				{
					value |= flag;
				}
				else
				{
					value &= ~flag;
				}

				*flags = static_cast<ImGuiWindowFlags_>(value);
				return true;
			}

			template<FlagsType flag>
			static inline void bind(Name name)
			{
				auto flags = NativeType<FlagsType>::instance();
				flags->bind(name, load<flag>, store<flag>, Property::Markup);
			}
		};
	}// namespace

#define trinex_ui_bind_flag(name)

	trinex_on_pre_init()
	{
		auto imgui = NativeType<ImGuiContext*>::instance();
		imgui->bind("window_pos", window_pos, Property::Markup);
		imgui->bind("window_size", window_size, Property::Markup);
		imgui->bind("window_width", window_width, Property::Markup);
		imgui->bind("window_height", window_height, Property::Markup);
		imgui->bind("content_size", content_size, Property::Markup);
		imgui->bind("content_min", content_min, Property::Markup);
		imgui->bind("content_max", content_max, Property::Markup);
		imgui->bind("cursor_pos", cursor_pos, Property::Markup);
		imgui->bind("cursor_screen_pos", cursor_screen_pos, Property::Markup);
		imgui->bind("item_min", item_min, Property::Markup);
		imgui->bind("item_max", item_max, Property::Markup);
		imgui->bind("item_size", item_size, Property::Markup);
		imgui->bind("scroll_x", scroll_x, Property::Markup);
		imgui->bind("scroll_y", scroll_y, Property::Markup);
		imgui->bind("scroll_max_x", scroll_max_x, Property::Markup);
		imgui->bind("scroll_max_y", scroll_max_y, Property::Markup);
		imgui->bind("mouse_pos", mouse_pos, Property::Markup);
		imgui->bind("font_size", font_size, Property::Markup);
		imgui->bind("frame_height", frame_height, Property::Markup);
		imgui->bind("frame_height_with_spacing", frame_height_with_spacing, Property::Markup);

		{
			using Binder = FlagsBinder<ImGuiWindowFlags, ImGuiWindowFlags_>;
			Binder::bind<ImGuiWindowFlags_ChildWindow>("child_window");
			Binder::bind<ImGuiWindowFlags_NoTitleBar>("no_title_bar");
			Binder::bind<ImGuiWindowFlags_NoResize>("no_resize");
			Binder::bind<ImGuiWindowFlags_NoMove>("no_move");
			Binder::bind<ImGuiWindowFlags_NoScrollbar>("no_scrollbar");
			Binder::bind<ImGuiWindowFlags_NoScrollWithMouse>("no_scroll_with_mouse");
			Binder::bind<ImGuiWindowFlags_NoCollapse>("no_collapse");
			Binder::bind<ImGuiWindowFlags_AlwaysAutoResize>("always_auto_resize");
			Binder::bind<ImGuiWindowFlags_NoBackground>("no_background");
			Binder::bind<ImGuiWindowFlags_NoSavedSettings>("no_saved_settings");
			Binder::bind<ImGuiWindowFlags_NoMouseInputs>("no_mouse_inputs");
			Binder::bind<ImGuiWindowFlags_MenuBar>("menu_bar");
			Binder::bind<ImGuiWindowFlags_HorizontalScrollbar>("horizontal_scrollbar");
			Binder::bind<ImGuiWindowFlags_NoFocusOnAppearing>("no_focus_on_appearing");
			Binder::bind<ImGuiWindowFlags_NoBringToFrontOnFocus>("no_bring_to_front_on_focus");
			Binder::bind<ImGuiWindowFlags_AlwaysVerticalScrollbar>("always_vertical_scrollbar");
			Binder::bind<ImGuiWindowFlags_AlwaysHorizontalScrollbar>("always_horizontal_scrollbar");
			Binder::bind<ImGuiWindowFlags_NoNavInputs>("no_nav_inputs");
			Binder::bind<ImGuiWindowFlags_NoNavFocus>("no_nav_focus");
			Binder::bind<ImGuiWindowFlags_UnsavedDocument>("unsaved_document");
			Binder::bind<ImGuiWindowFlags_NoDocking>("no_docking");
			Binder::bind<ImGuiWindowFlags_NoNav>("no_nav");
			Binder::bind<ImGuiWindowFlags_NoDecoration>("no_decoration");
			Binder::bind<ImGuiWindowFlags_NoInputs>("no_inputs");
		}
	}
}// namespace Trinex::UI::Refl
