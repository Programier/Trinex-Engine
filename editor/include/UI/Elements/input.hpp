#pragma once
#include <UI/Elements/frame.hpp>

namespace Trinex::UI
{
	class InputFloat : public Frame
	{
		trinex_ui_element(InputFloat, Frame);

	public:
		String label;
		f32 value     = 0.0f;
		String format = "%.3f";

		UpdateFlags on_begin_update() override;
	};

	class InputInt : public Frame
	{
		trinex_ui_element(InputInt, Frame);

	public:
		String label;
		i32 value = 0;

		UpdateFlags on_begin_update() override;
	};

	class InputText : public Frame
	{
		trinex_ui_element(InputText, Frame);

	public:
		String label;
		String hint;
		String value;
		Vec4 cursor_color      = {0.26f, 0.59f, 0.98f, 1.00f};
		Vec4 selected_bg_color = {0.26f, 0.59f, 0.98f, 0.35f};

		InputText& push_style() override;
		InputText& pop_style() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
