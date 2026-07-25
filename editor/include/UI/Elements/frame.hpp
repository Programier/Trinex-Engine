#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Frame : public Element
	{
		trinex_ui_element(Frame, Element);

	public:
		Vec2 padding    = {4.0f, 3.0f};
		f32 rounding    = 0.0f;
		f32 border_size = 0.0f;
		Vec4 background_color = {0.20f, 0.20f, 0.25f, 0.94f};

		Frame& push_style() override;
		Frame& pop_style() override;
	};
}// namespace Trinex::UI
