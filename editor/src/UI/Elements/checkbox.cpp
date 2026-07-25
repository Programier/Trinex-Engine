#include <UI/Elements/checkbox.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Checkbox)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags Checkbox::on_begin_update()
	{
		return item_state_flags(readback_if(UI::checkbox(label, &value)));
	}

	trinex_implement_ui_element(Toggle)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags Toggle::on_begin_update()
	{
		return item_state_flags(readback_if(UI::toggle(label, &value)));
	}
}// namespace Trinex::UI
