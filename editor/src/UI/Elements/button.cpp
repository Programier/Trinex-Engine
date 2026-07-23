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
		if (UI::button(label))
		{
			dispatch(on_click);
		}

		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
