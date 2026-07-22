#include <UI/Elements/button.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Button)
	{
		reflection()->bind("text", &This::text);
	}

	Element::UpdateFlags Button::on_begin_update()
	{
		if (UI::button(text))
		{
			return UpdateFlags::Readback;
		}

		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
