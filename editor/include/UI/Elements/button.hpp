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

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
