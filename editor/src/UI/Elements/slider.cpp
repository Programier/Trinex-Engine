#include <UI/Elements/slider.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(SliderFloat)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("min", &This::min);
		reflection()->bind("max", &This::max);
		reflection()->bind("format", &This::format);
	}

	Element::UpdateFlags SliderFloat::on_begin_update()
	{
		return item_state_flags(readback_if(UI::slider(label, &value, min, max, format.c_str())));
	}

	trinex_implement_ui_element(SliderInt)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("min", &This::min);
		reflection()->bind("max", &This::max);
		reflection()->bind("format", &This::format);
	}

	Element::UpdateFlags SliderInt::on_begin_update()
	{
		return item_state_flags(readback_if(UI::slider(label, &value, min, max, format.c_str())));
	}
}// namespace Trinex::UI
