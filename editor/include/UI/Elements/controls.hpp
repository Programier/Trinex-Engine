#pragma once
#include <UI/Elements/frame.hpp>

namespace Trinex::UI
{
	class SmallButton : public Frame
	{
		trinex_ui_element(SmallButton, Frame);

	public:
		String label;
		Name on_click;

		UpdateFlags on_begin_update() override;
	};

	class IconButton : public Frame
	{
		trinex_ui_element(IconButton, Frame);

	public:
		String icon;
		String label;
		Size size = Size(0.0f, 0.0f);
		Name on_click;

		UpdateFlags on_begin_update() override;
	};

	class GhostButton : public Frame
	{
		trinex_ui_element(GhostButton, Frame);

	public:
		String label;
		Size size = Size(0.0f, 0.0f);
		Name on_click;

		UpdateFlags on_begin_update() override;
	};

	class DangerButton : public Frame
	{
		trinex_ui_element(DangerButton, Frame);

	public:
		String label;
		Size size = Size(0.0f, 0.0f);
		Name on_click;

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

	class RadioButton : public Frame
	{
		trinex_ui_element(RadioButton, Frame);

	public:
		String label;
		i32 value  = 0;
		i32 option = 0;
		Name on_click;

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

		UpdateFlags on_begin_update() override;
	};

	class ProgressBar : public Element
	{
		trinex_ui_element(ProgressBar, Element);

	public:
		f32 value = 0.0f;
		Size size = Size(-1.0f, 0.0f);
		String overlay;

		UpdateFlags on_begin_update() override;
	};

	class Spinner : public Element
	{
		trinex_ui_element(Spinner, Element);

	public:
		String id;
		Unit radius    = Unit(8.0f);
		Unit thickness = Unit(2.0f);
		Vec4 color     = Vec4(0.0f, 0.0f, 0.0f, 0.0f);

		UpdateFlags on_begin_update() override;
	};

	class ColorEdit : public Frame
	{
		trinex_ui_element(ColorEdit, Frame);

	public:
		String label;
		Vec4 color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		bool alpha = true;

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
