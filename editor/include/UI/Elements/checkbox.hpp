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
		Vec4 check_color       = {0.28f, 0.59f, 0.92f, 1.00f};
		Vec4 selected_bg_color = {0.20f, 0.20f, 0.25f, 0.94f};

		Checkbox& push_style() override;
		Checkbox& pop_style() override;
		UpdateFlags on_begin_update() override;
	};

	class Toggle : public Frame
	{
		trinex_ui_element(Toggle, Frame);

	public:
		String label;
		bool value = false;
		Vec4 check_color       = {0.28f, 0.59f, 0.92f, 1.00f};
		Vec4 selected_bg_color = {0.20f, 0.20f, 0.25f, 0.94f};

		Toggle& push_style() override;
		Toggle& pop_style() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
