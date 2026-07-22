#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class SliderFloat : public Element
	{
		trinex_ui_element(SliderFloat, Element);

	public:
		String label;
		f32 value     = 0.0f;
		f32 min       = 0.0f;
		f32 max       = 1.0f;
		String format = "%.3f";

		UpdateFlags on_begin_update() override;
	};

	class SliderInt : public Element
	{
		trinex_ui_element(SliderInt, Element);

	public:
		String label;
		i32 value     = 0;
		i32 min       = 0;
		i32 max       = 100;
		String format = "%d";

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
