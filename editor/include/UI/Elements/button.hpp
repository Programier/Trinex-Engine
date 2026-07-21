#pragma once
#include <UI/element.hpp>
namespace Trinex::UI
{
	class Button : public Element
	{
		trinex_ui_element(Button, Element);

	public:
		String text;

		bool on_begin_render() override;
	};
}// namespace Trinex::UI
