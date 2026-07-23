#include <UI/Elements/controls.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	static Element::UpdateFlags handle_click(Element* element, bool clicked, Name event)
	{
		if (clicked)
		{
			element->dispatch(event);
		}

		return Element::readback_if(clicked);
	}

	trinex_implement_ui_element(SmallButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags SmallButton::on_begin_update()
	{
		return handle_click(this, UI::small_button(label), on_click);
	}

	trinex_implement_ui_element(IconButton)
	{
		reflection()->bind("icon", &This::icon);
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags IconButton::on_begin_update()
	{
		ButtonOptions options;
		options.size = size;
		return handle_click(this, UI::icon_button(icon, label, options), on_click);
	}

	trinex_implement_ui_element(GhostButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags GhostButton::on_begin_update()
	{
		return handle_click(this, UI::ghost_button(label, size), on_click);
	}

	trinex_implement_ui_element(DangerButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags DangerButton::on_begin_update()
	{
		return handle_click(this, UI::danger_button(label, size), on_click);
	}

	trinex_implement_ui_element(InvisibleButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags InvisibleButton::on_begin_update()
	{
		ButtonOptions options;
		options.size = size;
		return handle_click(this, UI::invisible_button(label, options), on_click);
	}

	trinex_implement_ui_element(RadioButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("option", &This::option);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags RadioButton::on_begin_update()
	{
		return handle_click(this, UI::radio_button(label, &value, option), on_click);
	}

	trinex_implement_ui_element(Selectable)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags Selectable::on_begin_update()
	{
		return handle_click(this, UI::selectable(label, selected, SelectableFlags::Undefined, size), on_click);
	}

	trinex_implement_ui_element(ProgressBar)
	{
		reflection()->bind("value", &This::value);
		reflection()->bind("size", &This::size);
		reflection()->bind("overlay", &This::overlay);
	}

	Element::UpdateFlags ProgressBar::on_begin_update()
	{
		UI::progress_bar(value, size, overlay);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Spinner)
	{
		reflection()->bind("id", &This::id);
		reflection()->bind("radius", &This::radius);
		reflection()->bind("thickness", &This::thickness);
		reflection()->bind("color", &This::color);
	}

	Element::UpdateFlags Spinner::on_begin_update()
	{
		UI::push_id(this);
		UI::spinner(id.empty() ? "##Spinner" : id, radius, thickness, color);
		UI::pop_id();
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(ColorEdit)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("color", &This::color);
		reflection()->bind("alpha", &This::alpha);
	}

	Element::UpdateFlags ColorEdit::on_begin_update()
	{
		return readback_if(UI::color_edit(label, &color, alpha));
	}
}// namespace Trinex::UI
