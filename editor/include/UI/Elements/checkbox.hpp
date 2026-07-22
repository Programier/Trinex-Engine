#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Checkbox : public Element
	{
		trinex_ui_element(Checkbox, Element);

	public:
		String label;
		bool value = false;

		bool on_begin_render() override;
	};

	class Toggle : public Element
	{
		trinex_ui_element(Toggle, Element);

	public:
		String label;
		bool value = false;

		bool on_begin_render() override;
	};
}// namespace Trinex::UI
