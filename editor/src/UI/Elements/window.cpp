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
	}

	Window& Window::push_style()
	{
		Super::push_style();

		push_style_var(ImGuiStyleVar_WindowPadding, padding);
		push_style_var(ImGuiStyleVar_WindowMinSize, min_size);
		push_style_var(ImGuiStyleVar_WindowTitleAlign, title_align);
		push_style_var(ImGuiStyleVar_WindowRounding, rounding);
		push_style_var(ImGuiStyleVar_WindowBorderSize, border_size);

		return *this;
	}

	Window& Window::pop_style()
	{
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
