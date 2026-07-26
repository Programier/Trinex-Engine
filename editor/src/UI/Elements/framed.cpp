#include <UI/Elements/framed.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(FramedGeometry)
	{
		trinex_ui_bind_property(padding);
		trinex_ui_bind_property(rounding);
		trinex_ui_bind_property(border_size);
	}

	trinex_implement_ui_element(Framed)
	{
		trinex_ui_bind_property(color);
	}

	FramedGeometry& FramedGeometry::push_style()
	{
		Super::push_style();

		push_style_var(ImGuiStyleVar_FramePadding, padding);
		push_style_var(ImGuiStyleVar_FrameRounding, rounding);
		push_style_var(ImGuiStyleVar_FrameBorderSize, border_size);

		return *this;
	}

	FramedGeometry& FramedGeometry::pop_style()
	{
		ImGui::PopStyleVar(3);
		return *Super::pop_style().as<This>();
	}


	Framed& Framed::push_style()
	{
		Super::push_style();

		push_style_color(ImGuiCol_FrameBg, color);
		push_style_color(ImGuiCol_FrameBgHovered, color);
		push_style_color(ImGuiCol_FrameBgActive, color);

		return *this;
	}

	Framed& Framed::pop_style()
	{
		ImGui::PopStyleColor(3);
		return *Super::pop_style().as<This>();
	}

}// namespace Trinex::UI
