#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Frame : public Element
	{
		trinex_ui_element(Frame, Element);

	public:
		Vec2 padding;
		f32 rounding;
		f32 border_size;

		Frame& push_style() override;
		Frame& pop_style() override;
	};
}// namespace Trinex::UI
