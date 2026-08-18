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
		Vec4 header_color = {0.20f, 0.25f, 0.30f, 1.00f};

		CollapsingHeader& push_scope() override;
		CollapsingHeader& pop_scope() override;
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
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
		Vec4 header_color = {0.20f, 0.25f, 0.30f, 1.00f};

		SectionHeader& push_scope() override;
		SectionHeader& pop_scope() override;
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class TreeNode : public Element
	{
		trinex_ui_element(TreeNode, Element);

	public:
		String label;
		String icon;
		String badge;
		bool default_open  = false;
		bool selected      = false;
		Unit line_size     = Unit(1.0f);
		Unit line_rounding = Unit(0.0f);
		Vec4 header_color  = {0.20f, 0.25f, 0.30f, 1.00f};
		Vec4 line_color    = {0.12f, 0.12f, 0.18f, 0.85f};

		TreeNode& push_scope() override;
		TreeNode& pop_scope() override;
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
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
		Unit rounding         = Unit(0.0f);
		Vec2 text_align       = {0.0f, 0.0f};
		Vec4 background_color = {0.20f, 0.25f, 0.30f, 1.00f};

		TreeLeaf& push_scope() override;
		TreeLeaf& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};

	class TabBar : public Element
	{
		trinex_ui_element(TabBar, Element);

	public:
		Unit border_size   = Unit(1.0f);
		Unit overline_size = Unit(2.0f);

		TabBar& push_scope() override;
		TabBar& pop_scope() override;
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};

	class Tab : public Element
	{
		trinex_ui_element(Tab, Element);

	public:
		String label;
		Size size = Size(0.0f, 0.0f);
		Name on_click;
		Unit rounding         = Unit(4.0f);
		Unit border_size      = Unit(0.0f);
		Unit min_width_base   = Unit(0.0f);
		Unit min_width_shrink = Unit(0.0f);
		Vec4 background_color = {0.11f, 0.15f, 0.17f, 1.00f};
		Vec4 overline_color   = {0.26f, 0.59f, 0.98f, 1.00f};

		Tab& push_scope() override;
		Tab& pop_scope() override;
		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
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
		Unit rounding         = Unit(0.0f);
		Vec2 text_align       = {0.0f, 0.0f};
		Vec4 background_color = {0.20f, 0.25f, 0.30f, 1.00f};

		SidebarItem& push_scope() override;
		SidebarItem& pop_scope() override;
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
		Unit rounding         = Unit(0.0f);
		Vec2 text_align       = {0.0f, 0.0f};
		Vec4 background_color = {0.20f, 0.25f, 0.30f, 1.00f};

		NavItem& push_scope() override;
		NavItem& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};

	class Breadcrumb : public Element
	{
		trinex_ui_element(Breadcrumb, Element);

	public:
		String label;
		bool current = false;
		Name on_click;
		Unit rounding         = Unit(0.0f);
		Vec2 text_align       = {0.0f, 0.0f};
		Vec4 background_color = {0.20f, 0.25f, 0.30f, 1.00f};

		Breadcrumb& push_scope() override;
		Breadcrumb& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
