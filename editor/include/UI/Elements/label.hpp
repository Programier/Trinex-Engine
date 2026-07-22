#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Label : public Element
	{
		trinex_ui_element(Label, Element);

	public:
		String text;
		String value;

		bool on_begin_render() override;
	};
}// namespace Trinex::UI
