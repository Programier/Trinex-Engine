#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Panel : public Element
	{
		trinex_ui_element(Panel, Element);

	public:
		String id;
		Size size       = Size(0.0f, 0.0f);
		bool border     = true;
		bool background = true;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class GroupPanel : public Element
	{
		trinex_ui_element(GroupPanel, Element);

	public:
		String label;
		Size size       = Size(0.0f, 0.0f);
		bool border     = true;
		bool background = true;

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
		Size size = Size(0.0f, 0.0f);
		f32 align = -1.0f;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class Vertical : public Element
	{
		trinex_ui_element(Vertical, Element);

	public:
		String id;
		Size size = Size(0.0f, 0.0f);
		f32 align = -1.0f;

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
		Size size   = Size(0.0f, 0.0f);
		bool border = false;

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
