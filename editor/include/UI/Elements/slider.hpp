#pragma once
#include <UI/Elements/frame.hpp>

namespace Trinex::UI
{
	class SliderFloat : public Frame
	{
		trinex_ui_element(SliderFloat, Frame);

	public:
		String label;
		f32 value     = 0.0f;
		f32 min       = 0.0f;
		f32 max       = 1.0f;
		String format = "%.3f";
		f32 grab_min_size = 12.0f;
		f32 grab_rounding = 0.0f;
		Vec4 grab_color   = {0.28f, 0.59f, 0.92f, 1.00f};

		SliderFloat& push_style() override;
		SliderFloat& pop_style() override;
		UpdateFlags on_begin_update() override;
	};

	class SliderInt : public Frame
	{
		trinex_ui_element(SliderInt, Frame);

	public:
		String label;
		i32 value     = 0;
		i32 min       = 0;
		i32 max       = 100;
		String format = "%d";
		f32 grab_min_size = 12.0f;
		f32 grab_rounding = 0.0f;
		Vec4 grab_color   = {0.28f, 0.59f, 0.92f, 1.00f};

		SliderInt& push_style() override;
		SliderInt& pop_style() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
