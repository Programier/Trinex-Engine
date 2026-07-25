#include <UI/Elements/button.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Button)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags Button::on_begin_update()
	{
		const bool clicked = UI::button(label);

		if (clicked)
		{
			dispatch(on_click);
		}

		return item_state_flags(readback_if(clicked));
	}
}// namespace Trinex::UI
