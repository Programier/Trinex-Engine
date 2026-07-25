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
	}

	Frame& Frame::push_style()
	{
		Super::push_style();

		push_style_var(ImGuiStyleVar_FramePadding, padding);
		push_style_var(ImGuiStyleVar_FrameRounding, rounding);
		push_style_var(ImGuiStyleVar_FrameBorderSize, border_size);

		return *this;
	}

	Frame& Frame::pop_style()
	{
		ImGui::PopStyleVar(3);
		return *Super::pop_style().as<This>();
	}

}// namespace Trinex::UI
