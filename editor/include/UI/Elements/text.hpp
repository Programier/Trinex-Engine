#pragma once

#include <UI/element.hpp>

namespace Trinex::UI
{
	class Text : public Element
	{
		trinex_ui_element(Text, Element);

	public:
		String text;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
