#include <UI/Elements/text.hpp>
#include <UI/reflection.hpp>
#include <imgui_internal.h>


namespace Trinex::UI
{
	trinex_implement_ui_element(Text)
	{
		trinex_ui_bind_property(text, Markup);
		trinex_ui_bind_property(color, Style);
	}

	Text& Text::push_style()
	{
		Super::push_style();
		push_style_color(ImGuiCol_Text, color);
		push_style_color(ImGuiCol_TextDisabled, color);
		push_style_color(ImGuiCol_TextSelectedBg, color);
		push_style_color(ImGuiCol_TextLink, color);
		return *this;
	}

	Text& Text::pop_style()
	{
		ImGui::PopStyleColor(4);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags Text::on_begin_update()
	{
		ImGui::TextEx(text.c_str(), text.c_str() + text.size(), 0);
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
