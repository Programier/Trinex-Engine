#pragma once
#include <UI/Elements/framed.hpp>

namespace Trinex::UI
{
	class CheckBox : public Framed
	{
		trinex_ui_element(CheckBox, Framed);

	private:
		f32 m_check_progress = -1.0f;

	public:
		String label;
		bool value                   = false;
		Vec4 check_color             = {0.28f, 0.59f, 0.92f, 1.00f};
		f32 check_animation_duration = 0.12f;
		Ease check_animation_ease    = Ease::OutCubic;

		CheckBox& push_scope() override;
		CheckBox& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
