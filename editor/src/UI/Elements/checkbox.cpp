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

	bool Checkbox::on_begin_render()
	{
		UI::checkbox(label, &value);
		return false;
	}

	trinex_implement_ui_element(Toggle)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	bool Toggle::on_begin_render()
	{
		UI::toggle(label, &value);
		return false;
	}
}// namespace Trinex::UI
