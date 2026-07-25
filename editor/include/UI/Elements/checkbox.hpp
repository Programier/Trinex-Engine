#pragma once
#include <UI/Elements/frame.hpp>

namespace Trinex::UI
{
	class Checkbox : public Frame
	{
		trinex_ui_element(Checkbox, Frame);

	public:
		String label;
		bool value = false;

		UpdateFlags on_begin_update() override;
	};

	class Toggle : public Frame
	{
		trinex_ui_element(Toggle, Frame);

	public:
		String label;
		bool value = false;

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
