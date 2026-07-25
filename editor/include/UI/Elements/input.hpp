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

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
