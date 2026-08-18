#pragma once
#include <UI/Elements/button.hpp>
#include <UI/Elements/framed.hpp>

namespace Trinex::UI
{
	class SmallButton : public Button
	{
		trinex_ui_element(SmallButton, Button);

	public:
		UpdateFlags on_begin_update() override;
	};

	class IconButton : public Button
	{
		trinex_ui_element(IconButton, Button);

	public:
		String icon;

		UpdateFlags on_begin_update() override;
	};

	class InvisibleButton : public Element
	{
		trinex_ui_element(InvisibleButton, Element);

	public:
		String label;
		Size size = Size(0.0f, 0.0f);
		Name on_click;

		UpdateFlags on_begin_update() override;
	};

	class RadioGroup : public Element
	{
		trinex_ui_element(RadioGroup, Element);

	private:
		static RadioGroup* s_current;

	public:
		i32 value                = 0;
		UpdateFlags update_flags = UpdateFlags::Undefined;

		static RadioGroup* current();

		RadioGroup& push_scope() override;
		RadioGroup& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};

	class RadioOption : public Framed
	{
		trinex_ui_element(RadioOption, Framed);

	private:
		f32 m_check_progress = -1.0f;

	public:
		String label;
		i32 option = 0;
		Name on_click;
		Vec4 check_color = {0.28f, 0.59f, 0.92f, 1.00f};
		f32 check_animation_duration = 0.12f;
		Ease check_animation_ease    = Ease::OutCubic;

		RadioOption& push_scope() override;
		RadioOption& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};

	class Selectable : public Element
	{
		trinex_ui_element(Selectable, Element);

	public:
		String label;
		bool selected = false;
		Size size     = Size(0.0f, 0.0f);
		Name on_click;
		Unit rounding         = Unit(0.0f);
		Vec2 text_align       = {0.0f, 0.0f};
		Vec4 background_color = {0.20f, 0.25f, 0.30f, 1.00f};

		Selectable& push_scope() override;
		Selectable& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};

	class ProgressBar : public Element
	{
		trinex_ui_element(ProgressBar, Element);

	public:
		f32 value = 0.0f;
		Size size = Size(-1.0f, 0.0f);
		String overlay;
		Vec4 color = {0.90f, 0.70f, 0.00f, 1.00f};

		ProgressBar& push_scope() override;
		ProgressBar& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};

	class ColorEdit : public Framed
	{
		trinex_ui_element(ColorEdit, Framed);

	public:
		String label;
		Vec4 value = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
