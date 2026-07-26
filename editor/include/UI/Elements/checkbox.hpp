#pragma once
#include <UI/Elements/framed.hpp>

namespace Trinex::UI
{
	class Checkbox : public Framed
	{
		trinex_ui_element(Checkbox, Framed);

	public:
		String label;
		bool value       = false;
		Vec4 check_color = {0.28f, 0.59f, 0.92f, 1.00f};

		Checkbox& push_style() override;
		Checkbox& pop_style() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
