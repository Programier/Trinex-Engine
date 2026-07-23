#include <UI/Elements/feedback.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(HelpMarker)
	{
		reflection()->bind("text", &This::text);
	}

	Element::UpdateFlags HelpMarker::on_begin_update()
	{
		UI::help_marker(text);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Tooltip)
	{
		reflection()->bind("text", &This::text);
		reflection()->bind("delay", &This::delay);
	}

	Element::UpdateFlags Tooltip::on_begin_update()
	{
		UI::tooltip_if_hovered(text, delay);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Badge)
	{
		reflection()->bind("text", &This::text);
		reflection()->bind("color", &This::color);
	}

	Element::UpdateFlags Badge::on_begin_update()
	{
		UI::badge(text, color);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Pill)
	{
		reflection()->bind("text", &This::text);
		reflection()->bind("color", &This::color);
	}

	Element::UpdateFlags Pill::on_begin_update()
	{
		UI::pill(text, color);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(StatusDot)
	{
		reflection()->bind("color", &This::color);
		reflection()->bind("radius", &This::radius);
	}

	Element::UpdateFlags StatusDot::on_begin_update()
	{
		UI::status_dot(color, radius);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(KeyValueRow)
	{
		reflection()->bind("key", &This::key);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags KeyValueRow::on_begin_update()
	{
		UI::key_value_row(key, value);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Callout)
	{
		reflection()->bind("title", &This::title);
		reflection()->bind("message", &This::message);
	}

	Element::UpdateFlags Callout::on_begin_update()
	{
		UI::callout(title, message);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Banner)
	{
		reflection()->bind("title", &This::title);
		reflection()->bind("message", &This::message);
		reflection()->bind("accent", &This::accent);
	}

	Element::UpdateFlags Banner::on_begin_update()
	{
		UI::banner(title, message, accent);
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
