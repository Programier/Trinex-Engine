#include <UI/Elements/button.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Button)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(size, Markup);
		trinex_ui_bind_property(on_click, Markup);
		trinex_ui_bind_property(text_align, Style);
		trinex_ui_bind_property(color, Style);
	}

	Button& Button::push_scope()
	{
		Super::push_scope();
		push_style_var(ImGuiStyleVar_ButtonTextAlign, text_align);
		push_style_color(ImGuiCol_Button, color);
		push_style_color(ImGuiCol_ButtonHovered, color);
		push_style_color(ImGuiCol_ButtonActive, color);
		return *this;
	}

	Button& Button::pop_scope()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags Button::on_begin_update()
	{
		const bool clicked = ImGui::Button(label.c_str(), resolve(size));

		if (clicked)
		{
			dispatch(on_click);
		}

		return item_state_flags(readback_if(clicked));
	}
}// namespace Trinex::UI
