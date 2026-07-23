#pragma once
#include <UI/element.hpp>
namespace Trinex::UI
{
	class Button : public Element
	{
		trinex_ui_element(Button, Element);

	public:
		String label;
		Name on_click;

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
