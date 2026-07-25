#include <UI/Elements/frame.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Frame)
	{
		trinex_ui_bind_property(padding);
		trinex_ui_bind_property(rounding);
		trinex_ui_bind_property(border_size);
		trinex_ui_bind_property(background_color);
	}

	Frame& Frame::push_style()
	{
		Super::push_style();

		push_style_var(ImGuiStyleVar_FramePadding, padding);
		push_style_var(ImGuiStyleVar_FrameRounding, rounding);
		push_style_var(ImGuiStyleVar_FrameBorderSize, border_size);
		push_style_color(ImGuiCol_FrameBg, background_color);
		push_style_color(ImGuiCol_FrameBgHovered, background_color);
		push_style_color(ImGuiCol_FrameBgActive, background_color);

		return *this;
	}

	Frame& Frame::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(3);
		return *Super::pop_style().as<This>();
	}

}// namespace Trinex::UI
