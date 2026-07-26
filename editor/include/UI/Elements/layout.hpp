#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Panel : public Element
	{
		trinex_ui_element(Panel, Element);

	public:
		String id;
		Size size             = Size(0.0f, 0.0f);
		bool border           = true;
		bool background       = true;
		Vec2 padding          = {8.0f, 8.0f};
		f32 rounding          = 0.0f;
		f32 border_size       = 1.0f;
		Vec4 background_color = {0.00f, 0.00f, 0.00f, 0.00f};

		Panel& push_style() override;
		Panel& pop_style() override;
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class GroupPanel : public Element
	{
		trinex_ui_element(GroupPanel, Element);

	public:
		String label;
		Size size             = Size(0.0f, 0.0f);
		bool border           = true;
		bool background       = true;
		Vec2 padding          = {8.0f, 8.0f};
		f32 rounding          = 0.0f;
		f32 border_size       = 1.0f;
		Vec4 background_color = {0.00f, 0.00f, 0.00f, 0.00f};

		GroupPanel& push_style() override;
		GroupPanel& pop_style() override;
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class Group : public Element
	{
		trinex_ui_element(Group, Element);

	public:
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class Horizontal : public Element
	{
		trinex_ui_element(Horizontal, Element);

	public:
		String id;
		ImVec2 size = ImVec2(0.0f, 0.0f);
		f32 align   = -1.0f;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class Vertical : public Element
	{
		trinex_ui_element(Vertical, Element);

	public:
		String id;
		ImVec2 size = ImVec2(0.0f, 0.0f);
		f32 align   = -1.0f;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class Disabled : public Element
	{
		trinex_ui_element(Disabled, Element);

	public:
		bool disabled = true;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class ScrollArea : public Element
	{
		trinex_ui_element(ScrollArea, Element);

	public:
		String id;
		Size size                 = Size(0.0f, 0.0f);
		bool border               = false;
		Vec2 padding              = {8.0f, 8.0f};
		f32 rounding              = 0.0f;
		f32 border_size           = 1.0f;
		Vec4 background_color     = {0.00f, 0.00f, 0.00f, 0.00f};
		Vec4 scrollbar_bg_color   = {0.02f, 0.02f, 0.02f, 0.53f};
		Vec4 scrollbar_grab_color = {0.31f, 0.31f, 0.31f, 1.00f};

		ScrollArea& push_style() override;
		ScrollArea& pop_style() override;
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class AnimatedArea : public Element
	{
		trinex_ui_element(AnimatedArea, Element);

	public:
		String id;
		bool visible = true;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class Separator : public Element
	{
		trinex_ui_element(Separator, Element);

	public:
		f32 size          = 1.0f;
		f32 text_border   = 3.0f;
		Vec2 text_align   = {0.0f, 0.5f};
		Vec2 text_padding = {20.0f, 3.0f};
		Vec4 color        = {0.12f, 0.12f, 0.18f, 0.85f};

		Separator& push_style() override;
		Separator& pop_style() override;
		UpdateFlags on_begin_update() override;
	};

	class Spacing : public Element
	{
		trinex_ui_element(Spacing, Element);

	public:
		Unit amount = Unit(-1.0f);

		UpdateFlags on_begin_update() override;
	};

	class NewLine : public Element
	{
		trinex_ui_element(NewLine, Element);

	public:
		UpdateFlags on_begin_update() override;
	};

	class SameLine : public Element
	{
		trinex_ui_element(SameLine, Element);

	public:
		f32 offset  = 0.0f;
		f32 spacing = -1.0f;

		UpdateFlags on_begin_update() override;
	};

	class Indent : public Element
	{
		trinex_ui_element(Indent, Element);

	public:
		Unit amount = Unit(0.0f);

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class Dummy : public Element
	{
		trinex_ui_element(Dummy, Element);

	public:
		Size size = Size(0.0f, 0.0f);

		UpdateFlags on_begin_update() override;
	};

	class Spring : public Element
	{
		trinex_ui_element(Spring, Element);

	public:
		f32 weight  = 1.0f;
		f32 spacing = -1.0f;

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
