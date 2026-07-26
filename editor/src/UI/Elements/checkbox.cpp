#include <UI/Elements/checkbox.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Checkbox)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		trinex_ui_bind_property(check_color);
	}

	Checkbox& Checkbox::push_style()
	{
		Super::push_style();
		push_style_color(ImGuiCol_CheckMark, check_color);
		push_style_color(ImGuiCol_CheckboxSelectedBg, color);
		return *this;
	}

	Checkbox& Checkbox::pop_style()
	{
		ImGui::PopStyleColor(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags Checkbox::on_begin_update()
	{
		return item_state_flags(readback_if(ImGui::Checkbox(label.c_str(), &value)));
	}

}// namespace Trinex::UI
