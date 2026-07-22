#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class InputFloat : public Element
	{
		trinex_ui_element(InputFloat, Element);

	public:
		String label;
		f32 value = 0.0f;
		String format = "%.3f";

		bool on_begin_render() override;
	};

	class InputInt : public Element
	{
		trinex_ui_element(InputInt, Element);

	public:
		String label;
		i32 value = 0;

		bool on_begin_render() override;
	};

	class InputText : public Element
	{
		trinex_ui_element(InputText, Element);

	public:
		String label;
		String hint;
		String value;

		bool on_begin_render() override;
	};
}// namespace Trinex::UI
