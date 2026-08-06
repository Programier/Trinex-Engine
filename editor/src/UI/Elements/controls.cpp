#include <Core/etl/stack.hpp>
#include <UI/Elements/controls.hpp>
#include <UI/Elements/document.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	static Element::UpdateFlags handle_click(Element* element, bool clicked, Name event)
	{
		if (clicked)
		{
			element->dispatch(event);
		}

		return Element::item_state_flags(Element::readback_if(clicked));
	}

	trinex_implement_ui_element(SmallButton) {}

	Element::UpdateFlags SmallButton::on_begin_update()
	{
		const bool clicked = ImGui::SmallButton(label.c_str());
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(IconButton)
	{
		trinex_ui_bind_property(icon, Markup);
	}

	Element::UpdateFlags IconButton::on_begin_update()
	{
		const String text  = icon.empty() ? label : icon + " " + label;
		const bool clicked = ImGui::Button(text.c_str(), resolve(size));
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(InvisibleButton)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(size, Markup);
		trinex_ui_bind_property(on_click, Markup);
	}

	Element::UpdateFlags InvisibleButton::on_begin_update()
	{
		const bool clicked = ImGui::InvisibleButton(label.c_str(), resolve(size));
		return handle_click(this, clicked, on_click);
	}

	RadioGroup* RadioGroup::s_current = nullptr;

	trinex_implement_ui_element(RadioGroup)
	{
		trinex_ui_bind_property(value, Markup);
	}

	RadioGroup* RadioGroup::current()
	{
		return s_current;
	}

	RadioGroup& RadioGroup::push_scope()
	{
		Super::push_scope();
		stack()->push<RadioGroup*>(s_current);
		s_current = this;

		return *this;
	}

	RadioGroup& RadioGroup::pop_scope()
	{
		s_current = *stack()->pop<RadioGroup*>();
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags RadioGroup::on_begin_update()
	{
		UpdateFlags result = UpdateFlags::Default | update_flags;
		update_flags       = UpdateFlags::Undefined;
		return result;
	}

	trinex_implement_ui_element(RadioOption)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(option, Markup);
		trinex_ui_bind_property(on_click, Markup);
		trinex_ui_bind_property(check_color, Style);
	}

	RadioOption& RadioOption::push_scope()
	{
		Super::push_scope();
		push_style_color(ImGuiCol_CheckMark, check_color);
		return *this;
	}

	RadioOption& RadioOption::pop_scope()
	{
		ImGui::PopStyleColor();
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags RadioOption::on_begin_update()
	{
		RadioGroup* group = RadioGroup::current();
		if (group == nullptr)
		{
			return item_state_flags();
		}

		const bool clicked = ImGui::RadioButton(label.c_str(), group->value == option);

		if (clicked)
		{
			group->value = option;
			group->update_flags |= UpdateFlags::Readback;
			dispatch(on_click);
		}

		return item_state_flags(readback_if(clicked));
	}

	trinex_implement_ui_element(Selectable)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(selected, Markup);
		trinex_ui_bind_property(size, Markup);
		trinex_ui_bind_property(on_click, Markup);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(text_align, Style);
		trinex_ui_bind_property(background_color, Style);
	}

	Selectable& Selectable::push_scope()
	{
		Super::push_scope();
		push_style_var(ImGuiStyleVar_SelectableRounding, rounding);
		push_style_var(ImGuiStyleVar_SelectableTextAlign, text_align);
		push_style_color(ImGuiCol_Header, background_color);
		push_style_color(ImGuiCol_HeaderHovered, background_color);
		push_style_color(ImGuiCol_HeaderActive, background_color);
		return *this;
	}

	Selectable& Selectable::pop_scope()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags Selectable::on_begin_update()
	{
		const bool clicked = ImGui::Selectable(label.c_str(), selected, 0, resolve(size));
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(ProgressBar)
	{
		trinex_ui_bind_property(value, Markup);
		trinex_ui_bind_property(size, Markup);
		trinex_ui_bind_property(overlay, Markup);
		trinex_ui_bind_property(color, Style);
	}

	ProgressBar& ProgressBar::push_scope()
	{
		Super::push_scope();
		push_style_color(ImGuiCol_PlotHistogram, color);
		push_style_color(ImGuiCol_PlotHistogramHovered, color);
		return *this;
	}

	ProgressBar& ProgressBar::pop_scope()
	{
		ImGui::PopStyleColor(2);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags ProgressBar::on_begin_update()
	{
		ImGui::ProgressBar(value, resolve(size), overlay.empty() ? nullptr : overlay.c_str());
		return item_state_flags();
	}

	trinex_implement_ui_element(ColorEdit)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(value, Markup);
	}

	Element::UpdateFlags ColorEdit::on_begin_update()
	{
		const bool edited = ImGui::ColorEdit4(label.c_str(), &value.x);
		return item_state_flags(readback_if(edited));
	}
}// namespace Trinex::UI
