#include <UI/Elements/navigation.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	static Element::UpdateFlags handle_click(Element* element, bool clicked, const Name& event)
	{
		if (clicked)
		{
			element->dispatch(event);
		}

		return Element::readback_if(clicked);
	}

	static HeaderOptions header_options(StringView icon, StringView right_text, bool default_open, bool disabled)
	{
		HeaderOptions options;
		options.icon         = icon;
		options.right_text   = right_text;
		options.default_open = default_open;
		options.disabled     = disabled;
		return options;
	}

	static TreeNodeOptions tree_options(StringView icon, StringView badge, bool default_open, bool selected, bool leaf)
	{
		TreeNodeOptions options;
		options.icon         = icon;
		options.badge        = badge;
		options.default_open = default_open;
		options.selected     = selected;
		options.leaf         = leaf;
		return options;
	}

	trinex_implement_ui_element(CollapsingHeader)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("right_text", &This::right_text);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("disabled", &This::disabled);
	}

	Element::UpdateFlags CollapsingHeader::on_begin_update()
	{
		if (UI::begin_collapsing_header(label, header_options(icon, right_text, default_open, disabled)))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::End;
	}

	Element& CollapsingHeader::on_end_update()
	{
		UI::end_collapsing_header();
		return *this;
	}

	trinex_implement_ui_element(SectionHeader)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("right_text", &This::right_text);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("disabled", &This::disabled);
	}

	Element::UpdateFlags SectionHeader::on_begin_update()
	{
		if (UI::begin_section_header(label, header_options(icon, right_text, default_open, disabled)))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::End;
	}

	Element& SectionHeader::on_end_update()
	{
		UI::end_section_header();
		return *this;
	}

	trinex_implement_ui_element(TreeNode)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("selected", &This::selected);
	}

	Element::UpdateFlags TreeNode::on_begin_update()
	{
		if (UI::tree_node(label, tree_options(icon, badge, default_open, selected, false)))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::End;
	}

	Element& TreeNode::on_end_update()
	{
		UI::tree_pop();
		return *this;
	}

	trinex_implement_ui_element(TreeLeaf)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags TreeLeaf::on_begin_update()
	{
		return handle_click(this, UI::selectable_tree_item(label, selected, tree_options(icon, badge, false, selected, true)),
		                    on_click);
	}

	trinex_implement_ui_element(TabBar)
	{
		reflection()->bind("id", &This::id);
	}

	Element::UpdateFlags TabBar::on_begin_update()
	{
		UI::push_id(this);
		if (UI::begin_tab_bar(id.empty() ? "##TabBar" : id))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::End;
	}

	Element& TabBar::on_end_update()
	{
		UI::end_tab_bar();
		UI::pop_id();
		return *this;
	}

	trinex_implement_ui_element(Tab)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags Tab::on_begin_update()
	{
		return handle_click(this, UI::tab(label, selected, size), on_click);
	}

	trinex_implement_ui_element(SidebarItem)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags SidebarItem::on_begin_update()
	{
		return handle_click(this, UI::sidebar_item(label, selected, icon, badge), on_click);
	}

	trinex_implement_ui_element(NavItem)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags NavItem::on_begin_update()
	{
		return handle_click(this, UI::nav_item(label, selected, icon), on_click);
	}

	trinex_implement_ui_element(Breadcrumb)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("current", &This::current);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags Breadcrumb::on_begin_update()
	{
		return handle_click(this, UI::breadcrumb(label, current), on_click);
	}
}// namespace Trinex::UI
