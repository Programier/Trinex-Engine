#include <UI/Elements/layout.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	static PanelOptions panel_options(Size size, bool border, bool background)
	{
		PanelOptions options;
		options.size       = size;
		options.border     = border;
		options.background = background;
		return options;
	}

	trinex_implement_ui_element(Panel)
	{
		reflection()->bind("id", &This::id);
		reflection()->bind("size", &This::size);
		reflection()->bind("border", &This::border);
		reflection()->bind("background", &This::background);
	}

	Element::UpdateFlags Panel::on_begin_update()
	{
		UI::push_id(this);
		const bool visible = UI::begin_panel(id.empty() ? "##Panel" : id, panel_options(size, border, background));
		if (!visible)
		{
			UI::pop_id();
			return UpdateFlags::Undefined;
		}
		return UpdateFlags::Default;
	}

	Element& Panel::on_end_update()
	{
		UI::end_panel();
		UI::pop_id();
		return *this;
	}

	trinex_implement_ui_element(GroupPanel)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("border", &This::border);
		reflection()->bind("background", &This::background);
	}

	Element::UpdateFlags GroupPanel::on_begin_update()
	{
		UI::push_id(this);
		const bool visible = UI::begin_group_panel(label, panel_options(size, border, background));
		if (!visible)
		{
			UI::pop_id();
			return UpdateFlags::Undefined;
		}
		return UpdateFlags::Default;
	}

	Element& GroupPanel::on_end_update()
	{
		UI::end_group_panel();
		UI::pop_id();
		return *this;
	}

	trinex_implement_ui_element(Group) {}

	Element::UpdateFlags Group::on_begin_update()
	{
		UI::begin_group();
		return UpdateFlags::Default;
	}

	Element& Group::on_end_update()
	{
		UI::end_group();
		return *this;
	}

	trinex_implement_ui_element(Horizontal)
	{
		reflection()->bind("id", &This::id);
		reflection()->bind("size", &This::size);
		reflection()->bind("align", &This::align);
	}

	Element::UpdateFlags Horizontal::on_begin_update()
	{
		if (id.empty())
		{
			UI::begin_horizontal(this, size, align);
		}
		else
		{
			UI::begin_horizontal(id, size, align);
		}

		return UpdateFlags::Default;
	}

	Element& Horizontal::on_end_update()
	{
		UI::end_horizontal();
		return *this;
	}

	trinex_implement_ui_element(Vertical)
	{
		reflection()->bind("id", &This::id);
		reflection()->bind("size", &This::size);
		reflection()->bind("align", &This::align);
	}

	Element::UpdateFlags Vertical::on_begin_update()
	{
		if (id.empty())
		{
			UI::begin_vertical(this, size, align);
		}
		else
		{
			UI::begin_vertical(id, size, align);
		}

		return UpdateFlags::Default;
	}

	Element& Vertical::on_end_update()
	{
		UI::end_vertical();
		return *this;
	}

	trinex_implement_ui_element(Disabled)
	{
		reflection()->bind("disabled", &This::disabled);
	}

	Element::UpdateFlags Disabled::on_begin_update()
	{
		UI::begin_disabled(disabled);
		return UpdateFlags::Default;
	}

	Element& Disabled::on_end_update()
	{
		UI::end_disabled();
		return *this;
	}

	trinex_implement_ui_element(ScrollArea)
	{
		reflection()->bind("id", &This::id);
		reflection()->bind("size", &This::size);
		reflection()->bind("border", &This::border);
	}

	Element::UpdateFlags ScrollArea::on_begin_update()
	{
		UI::push_id(this);
		const bool visible = UI::begin_scroll_area(id.empty() ? "##ScrollArea" : id, size, border);
		if (!visible)
		{
			UI::pop_id();
			return UpdateFlags::Undefined;
		}
		return UpdateFlags::Default;
	}

	Element& ScrollArea::on_end_update()
	{
		UI::end_scroll_area();
		UI::pop_id();
		return *this;
	}

	trinex_implement_ui_element(AnimatedArea)
	{
		reflection()->bind("id", &This::id);
		reflection()->bind("visible", &This::visible);
	}

	Element::UpdateFlags AnimatedArea::on_begin_update()
	{
		UI::push_id(this);
		const bool opened = UI::begin_animated_area(id.empty() ? "##AnimatedArea" : id, visible);
		if (!opened)
		{
			UI::pop_id();
			return UpdateFlags::Undefined;
		}
		return UpdateFlags::Default;
	}

	Element& AnimatedArea::on_end_update()
	{
		UI::end_animated_area();
		UI::pop_id();
		return *this;
	}

	trinex_implement_ui_element(Separator) {}

	Element::UpdateFlags Separator::on_begin_update()
	{
		UI::separator();
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Spacing)
	{
		reflection()->bind("amount", &This::amount);
	}

	Element::UpdateFlags Spacing::on_begin_update()
	{
		UI::spacing(amount);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(NewLine) {}

	Element::UpdateFlags NewLine::on_begin_update()
	{
		UI::new_line();
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(SameLine)
	{
		reflection()->bind("offset", &This::offset);
		reflection()->bind("spacing", &This::spacing);
	}

	Element::UpdateFlags SameLine::on_begin_update()
	{
		UI::same_line(offset, spacing);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Indent)
	{
		reflection()->bind("amount", &This::amount);
	}

	Element::UpdateFlags Indent::on_begin_update()
	{
		UI::indent(amount);
		return UpdateFlags::Default;
	}

	Element& Indent::on_end_update()
	{
		UI::unindent(amount);
		return *this;
	}

	trinex_implement_ui_element(Dummy)
	{
		reflection()->bind("size", &This::size);
	}

	Element::UpdateFlags Dummy::on_begin_update()
	{
		UI::dummy(size);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Spring)
	{
		reflection()->bind("weight", &This::weight);
		reflection()->bind("spacing", &This::spacing);
	}

	Element::UpdateFlags Spring::on_begin_update()
	{
		UI::spring(weight, spacing);
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
