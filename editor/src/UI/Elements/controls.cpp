#include <UI/Elements/controls.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(SmallButton)
	{
		reflection()->bind("text", &This::text);
	}

	Element::UpdateFlags SmallButton::on_begin_update()
	{
		return readback_if(UI::small_button(text));
	}

	trinex_implement_ui_element(IconButton)
	{
		reflection()->bind("icon", &This::icon);
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
	}

	Element::UpdateFlags IconButton::on_begin_update()
	{
		ButtonOptions options;
		options.size = size;
		return readback_if(UI::icon_button(icon, label, options));
	}

	trinex_implement_ui_element(GhostButton)
	{
		reflection()->bind("text", &This::text);
		reflection()->bind("size", &This::size);
	}

	Element::UpdateFlags GhostButton::on_begin_update()
	{
		return readback_if(UI::ghost_button(text, size));
	}

	trinex_implement_ui_element(DangerButton)
	{
		reflection()->bind("text", &This::text);
		reflection()->bind("size", &This::size);
	}

	Element::UpdateFlags DangerButton::on_begin_update()
	{
		return readback_if(UI::danger_button(text, size));
	}

	trinex_implement_ui_element(InvisibleButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
	}

	Element::UpdateFlags InvisibleButton::on_begin_update()
	{
		ButtonOptions options;
		options.size = size;
		return readback_if(UI::invisible_button(label, options));
	}

	trinex_implement_ui_element(RadioButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("active", &This::active);
	}

	Element::UpdateFlags RadioButton::on_begin_update()
	{
		return readback_if(UI::radio_button(label, &value, active));
	}

	trinex_implement_ui_element(Selectable)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("size", &This::size);
	}

	Element::UpdateFlags Selectable::on_begin_update()
	{
		return readback_if(UI::selectable(label, selected, SelectableFlags::Undefined, size));
	}

	trinex_implement_ui_element(ProgressBar)
	{
		reflection()->bind("fraction", &This::fraction);
		reflection()->bind("size", &This::size);
		reflection()->bind("overlay", &This::overlay);
	}

	Element::UpdateFlags ProgressBar::on_begin_update()
	{
		UI::progress_bar(fraction, size, overlay);
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
