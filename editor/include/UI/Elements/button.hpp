#pragma once
#include <UI/Elements/frame.hpp>

namespace Trinex::UI
{
	class Button : public Frame
	{
		trinex_ui_element(Button, Frame);

	public:
		String label;
		Name on_click;
		Vec2 text_align = {0.5f, 0.5f};
		Vec4 background_color = {0.20f, 0.25f, 0.30f, 0.94f};

		Button& push_style() override;
		Button& pop_style() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
