#include <UI/Styles/tab.hpp>
#include <UI/element.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	TabStyle& TabStyle::push()
	{
		Element::push_style_var(ImGuiStyleVar_TabRounding, rounding);
		Element::push_style_var(ImGuiStyleVar_TabBorderSize, border_size);
		Element::push_style_var(ImGuiStyleVar_TabMinWidthBase, min_width_base);
		Element::push_style_var(ImGuiStyleVar_TabMinWidthShrink, min_width_shrink);
		Element::push_style_color(ImGuiCol_Tab, background_color);
		Element::push_style_color(ImGuiCol_TabSelectedOverline, overline_color);
		Element::push_style_color(ImGuiCol_TabDimmedSelectedOverline, overline_color);
		return *this;
	}

	TabStyle& TabStyle::pop()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(4);
		return *this;
	}

	trinex_on_pre_init()
	{
		auto type = Refl::NativeType<TabStyle>::instance();
		type->bind("rounding", &TabStyle::rounding, Refl::Property::Style);
		type->bind("border_size", &TabStyle::border_size, Refl::Property::Style);
		type->bind("min_width_base", &TabStyle::min_width_base, Refl::Property::Style);
		type->bind("min_width_shrink", &TabStyle::min_width_shrink, Refl::Property::Style);
		type->bind("background_color", &TabStyle::background_color, Refl::Property::Style);
		type->bind("overline_color", &TabStyle::overline_color, Refl::Property::Style);
		trinex_ui_bind_type_name(TabStyle);
	}
}// namespace Trinex::UI
