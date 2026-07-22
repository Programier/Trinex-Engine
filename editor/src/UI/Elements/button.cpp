#include <UI/Elements/button.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Button)
	{
		reflection()->bind("text", &This::text);
	}

	bool Button::on_begin_render()
	{
		if (UI::button(text))
		{
		}
		return false;
	}
}// namespace Trinex::UI
