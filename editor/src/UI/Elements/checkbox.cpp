#include <UI/Elements/checkbox.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Checkbox)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags Checkbox::on_begin_update()
	{
		return item_state_flags(readback_if(ImGui::Checkbox(label.c_str(), &value)));
	}

	trinex_implement_ui_element(Toggle)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags Toggle::on_begin_update()
	{
		return item_state_flags(readback_if(ImGui::Checkbox(label.c_str(), &value)));
	}
}// namespace Trinex::UI
