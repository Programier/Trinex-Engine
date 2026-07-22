#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class InputFloat : public Element
	{
		trinex_ui_element(InputFloat, Element);

	public:
		String label;
		f32 value     = 0.0f;
		String format = "%.3f";

		UpdateFlags on_begin_update() override;
	};

	class InputInt : public Element
	{
		trinex_ui_element(InputInt, Element);

	public:
		String label;
		i32 value = 0;

		UpdateFlags on_begin_update() override;
	};

	class InputText : public Element
	{
		trinex_ui_element(InputText, Element);

	public:
		String label;
		String hint;
		String value;

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
