#pragma once
#include <UI/Elements/framed.hpp>

namespace Trinex::UI
{
	class SliderFloat : public Framed
	{
		trinex_ui_element(SliderFloat, Framed);

	public:
		String label;
		f32 value          = 0.0f;
		f32 min            = 0.0f;
		f32 max            = 1.0f;
		String format      = "%.3f";
		Unit grab_min_size = Unit(12.0f);
		Unit grab_rounding = Unit(0.0f);
		Vec4 grab_color    = {0.28f, 0.59f, 0.92f, 1.00f};

		SliderFloat& push_scope() override;
		SliderFloat& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};

	class SliderInt : public Framed
	{
		trinex_ui_element(SliderInt, Framed);

	public:
		String label;
		i32 value          = 0;
		i32 min            = 0;
		i32 max            = 100;
		String format      = "%d";
		Unit grab_min_size = Unit(12.0f);
		Unit grab_rounding = Unit(0.0f);
		Vec4 grab_color    = {0.28f, 0.59f, 0.92f, 1.00f};

		SliderInt& push_scope() override;
		SliderInt& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
