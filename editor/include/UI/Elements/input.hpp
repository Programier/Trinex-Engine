#pragma once
#include <UI/Elements/framed.hpp>

namespace Trinex::UI
{
	class InputFloat : public Framed
	{
		trinex_ui_element(InputFloat, Framed);

	public:
		String label;
		f32 value     = 0.0f;
		String format = "%.3f";

		UpdateFlags on_begin_update() override;
	};

	class InputInt : public Framed
	{
		trinex_ui_element(InputInt, Framed);

	public:
		String label;
		i32 value = 0;

		UpdateFlags on_begin_update() override;
	};

	class InputText : public Framed
	{
		trinex_ui_element(InputText, Framed);

	public:
		String label;
		String hint;
		String value;
		Vec4 cursor_color = {0.26f, 0.59f, 0.98f, 1.00f};

		InputText& push_style() override;
		InputText& pop_style() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
