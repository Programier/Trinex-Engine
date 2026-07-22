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

	bool SliderFloat::on_begin_render()
	{
		UI::slider(label, &value, min, max, format.c_str());
		return false;
	}

	trinex_implement_ui_element(SliderInt)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("min", &This::min);
		reflection()->bind("max", &This::max);
		reflection()->bind("format", &This::format);
	}

	bool SliderInt::on_begin_render()
	{
		UI::slider(label, &value, min, max, format.c_str());
		return false;
	}
}// namespace Trinex::UI
