#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class CollapsingHeader : public Element
	{
		trinex_ui_element(CollapsingHeader, Element);

	public:
		String label;
		String icon;
		String right_text;
		bool default_open = false;
		bool disabled     = false;

		UpdateFlags on_begin_update() override;
		Element& on_end_update() override;
	};

	class SectionHeader : public Element
	{
		trinex_ui_element(SectionHeader, Element);

	public:
		String label;
		String icon;
		String right_text;
		bool default_open = false;
		bool disabled     = false;

		UpdateFlags on_begin_update() override;
		Element& on_end_update() override;
	};

	class TreeNode : public Element
	{
		trinex_ui_element(TreeNode, Element);

	public:
		String label;
		String icon;
		String badge;
		bool default_open = false;
		bool selected     = false;

		UpdateFlags on_begin_update() override;
		Element& on_end_update() override;
	};

	class TreeLeaf : public Element
	{
		trinex_ui_element(TreeLeaf, Element);

	public:
		String label;
		String icon;
		String badge;
		bool selected = false;
		Name on_click;

		UpdateFlags on_begin_update() override;
	};

	class TabBar : public Element
	{
		trinex_ui_element(TabBar, Element);

	public:
		String id;

		UpdateFlags on_begin_update() override;
		Element& on_end_update() override;
	};

	class Tab : public Element
	{
		trinex_ui_element(Tab, Element);

	public:
		String label;
		bool selected = false;
		Size size     = Size(0.0f, 0.0f);
		Name on_click;

		UpdateFlags on_begin_update() override;
	};

	class SidebarItem : public Element
	{
		trinex_ui_element(SidebarItem, Element);

	public:
		String label;
		String icon;
		String badge;
		bool selected = false;
		Name on_click;

		UpdateFlags on_begin_update() override;
	};

	class NavItem : public Element
	{
		trinex_ui_element(NavItem, Element);

	public:
		String label;
		String icon;
		bool selected = false;
		Name on_click;

		UpdateFlags on_begin_update() override;
	};

	class Breadcrumb : public Element
	{
		trinex_ui_element(Breadcrumb, Element);

	public:
		String label;
		bool current = false;
		Name on_click;

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
