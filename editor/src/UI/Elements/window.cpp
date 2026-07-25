#include <UI/Elements/window.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Window)
	{
		trinex_ui_bind_property(title);
		trinex_ui_bind_property(padding);
		trinex_ui_bind_property(min_size);
		trinex_ui_bind_property(title_align);
		trinex_ui_bind_property(rounding);
		trinex_ui_bind_property(border_size);
		trinex_ui_bind_property(background_color);
		trinex_ui_bind_property(title_color);
		trinex_ui_bind_property(resize_grip_color);
	}

	Window& Window::push_style()
	{
		Super::push_style();

		push_style_var(ImGuiStyleVar_WindowPadding, padding);
		push_style_var(ImGuiStyleVar_WindowMinSize, min_size);
		push_style_var(ImGuiStyleVar_WindowTitleAlign, title_align);
		push_style_var(ImGuiStyleVar_WindowRounding, rounding);
		push_style_var(ImGuiStyleVar_WindowBorderSize, border_size);
		push_style_color(ImGuiCol_WindowBg, background_color);
		push_style_color(ImGuiCol_TitleBg, title_color);
		push_style_color(ImGuiCol_TitleBgActive, title_color);
		push_style_color(ImGuiCol_TitleBgCollapsed, title_color);
		push_style_color(ImGuiCol_ResizeGrip, resize_grip_color);
		push_style_color(ImGuiCol_ResizeGripHovered, resize_grip_color);
		push_style_color(ImGuiCol_ResizeGripActive, resize_grip_color);

		return *this;
	}

	Window& Window::pop_style()
	{
		ImGui::PopStyleColor(7);
		ImGui::PopStyleVar(5);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags Window::on_begin_update()
	{
		if (title.empty())
			return UpdateFlags::Undefined;

		UpdateFlags flags = UpdateFlags::End;

		if (ImGui::Begin(title.c_str()))
		{
			flags |= UpdateFlags::Childs;

			if (ImGui::IsWindowFocused())
			{
				flags |= UpdateFlags::Focused;
			}

			if (ImGui::IsWindowHovered())
			{
				flags |= UpdateFlags::Hovered;
			}
		}

		return flags;
	}

	Element& Window::on_end_update(UpdateFlags flags)
	{
		ImGui::End();
		return *this;
	}
}// namespace Trinex::UI
