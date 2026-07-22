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

		UpdateFlags on_begin_update() override;
	};

	class Toggle : public Element
	{
		trinex_ui_element(Toggle, Element);

	public:
		String label;
		bool value = false;

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
