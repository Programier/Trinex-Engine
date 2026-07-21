#pragma once

#include <UI/element.hpp>

namespace Trinex::UI
{
	class Text : public Element
	{
		trinex_ui_element(Text, Element);

	public:
		String text;
		bool on_begin_render() override;
	};
}// namespace Trinex::UI
