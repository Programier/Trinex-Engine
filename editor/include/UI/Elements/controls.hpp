// #pragma once
// #include <UI/Elements/visual.hpp>

// namespace Trinex::UI
// {
// 	class SmallButton : public Visual
// 	{
// 		trinex_ui_element(SmallButton, Visual);

// 	public:
// 		String label;
// 		Name on_click;
// 		Vec2 text_align = {0.5f, 0.5f};
// 		Vec4 color      = {0.20f, 0.25f, 0.30f, 0.94f};

// 		SmallButton& push_style() override;
// 		SmallButton& pop_style() override;
// 		UpdateFlags on_begin_update() override;
// 	};

// 	class IconButton : public Visual
// 	{
// 		trinex_ui_element(IconButton, Visual);

// 	public:
// 		String icon;
// 		String label;
// 		Size size = Size(0.0f, 0.0f);
// 		Name on_click;
// 		Vec2 text_align = {0.5f, 0.5f};
// 		Vec4 color      = {0.20f, 0.25f, 0.30f, 0.94f};

// 		IconButton& push_style() override;
// 		IconButton& pop_style() override;
// 		UpdateFlags on_begin_update() override;
// 	};

// 	class GhostButton : public Visual
// 	{
// 		trinex_ui_element(GhostButton, Frame);

// 	public:
// 		String label;
// 		Size size = Size(0.0f, 0.0f);
// 		Name on_click;
// 		Vec2 text_align       = {0.5f, 0.5f};
// 		Vec4 background_color = {0.20f, 0.25f, 0.30f, 0.94f};

// 		GhostButton& push_style() override;
// 		GhostButton& pop_style() override;
// 		UpdateFlags on_begin_update() override;
// 	};

// 	class DangerButton : public Frame
// 	{
// 		trinex_ui_element(DangerButton, Frame);

// 	public:
// 		String label;
// 		Size size = Size(0.0f, 0.0f);
// 		Name on_click;
// 		Vec2 text_align       = {0.5f, 0.5f};
// 		Vec4 background_color = {0.55f, 0.12f, 0.12f, 1.0f};

// 		DangerButton& push_style() override;
// 		DangerButton& pop_style() override;
// 		UpdateFlags on_begin_update() override;
// 	};

// 	class InvisibleButton : public Element
// 	{
// 		trinex_ui_element(InvisibleButton, Element);

// 	public:
// 		String label;
// 		Size size = Size(0.0f, 0.0f);
// 		Name on_click;

// 		UpdateFlags on_begin_update() override;
// 	};

// 	class RadioButton : public Frame
// 	{
// 		trinex_ui_element(RadioButton, Frame);

// 	public:
// 		String label;
// 		i32 value  = 0;
// 		i32 option = 0;
// 		Name on_click;
// 		Vec4 check_color = {0.28f, 0.59f, 0.92f, 1.00f};

// 		RadioButton& push_style() override;
// 		RadioButton& pop_style() override;
// 		UpdateFlags on_begin_update() override;
// 	};

// 	class Selectable : public Element
// 	{
// 		trinex_ui_element(Selectable, Element);

// 	public:
// 		String label;
// 		bool selected = false;
// 		Size size     = Size(0.0f, 0.0f);
// 		Name on_click;
// 		f32 rounding          = 0.0f;
// 		Vec2 text_align       = {0.0f, 0.0f};
// 		Vec4 background_color = {0.20f, 0.25f, 0.30f, 1.00f};

// 		Selectable& push_style() override;
// 		Selectable& pop_style() override;
// 		UpdateFlags on_begin_update() override;
// 	};

// 	class ProgressBar : public Element
// 	{
// 		trinex_ui_element(ProgressBar, Element);

// 	public:
// 		f32 value = 0.0f;
// 		Size size = Size(-1.0f, 0.0f);
// 		String overlay;
// 		Vec4 color = {0.90f, 0.70f, 0.00f, 1.00f};

// 		ProgressBar& push_style() override;
// 		ProgressBar& pop_style() override;
// 		UpdateFlags on_begin_update() override;
// 	};

// 	class Spinner : public Element
// 	{
// 		trinex_ui_element(Spinner, Element);

// 	public:
// 		String id;
// 		Unit radius    = Unit(8.0f);
// 		Unit thickness = Unit(2.0f);
// 		Vec4 color     = Vec4(0.0f, 0.0f, 0.0f, 0.0f);

// 		UpdateFlags on_begin_update() override;
// 	};

// 	class ColorEdit : public Frame
// 	{
// 		trinex_ui_element(ColorEdit, Frame);

// 	public:
// 		String label;
// 		Vec4 color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
// 		bool alpha = true;

// 		UpdateFlags on_begin_update() override;
// 	};
// }// namespace Trinex::UI
