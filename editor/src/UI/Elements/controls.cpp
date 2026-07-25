#include <UI/Elements/controls.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	static ImVec2 to_imgui_size(Size size)
	{
		return ImVec2(size.width.value, size.height.value);
	}

	static Element::UpdateFlags handle_click(Element* element, bool clicked, Name event)
	{
		if (clicked)
		{
			element->dispatch(event);
		}

		return Element::item_state_flags(Element::readback_if(clicked));
	}

	trinex_implement_ui_element(SmallButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(text_align);
		trinex_ui_bind_property(background_color);
	}

	SmallButton& SmallButton::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_ButtonTextAlign, text_align);
		push_style_color(ImGuiCol_Button, background_color);
		push_style_color(ImGuiCol_ButtonHovered, background_color);
		push_style_color(ImGuiCol_ButtonActive, background_color);
		return *this;
	}

	SmallButton& SmallButton::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags SmallButton::on_begin_update()
	{
		return handle_click(this, ImGui::SmallButton(label.c_str()), on_click);
	}

	trinex_implement_ui_element(IconButton)
	{
		reflection()->bind("icon", &This::icon);
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(text_align);
		trinex_ui_bind_property(background_color);
	}

	IconButton& IconButton::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_ButtonTextAlign, text_align);
		push_style_color(ImGuiCol_Button, background_color);
		push_style_color(ImGuiCol_ButtonHovered, background_color);
		push_style_color(ImGuiCol_ButtonActive, background_color);
		return *this;
	}

	IconButton& IconButton::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags IconButton::on_begin_update()
	{
		const String text = icon.empty() ? label : icon + " " + label;
		return handle_click(this, ImGui::Button(text.c_str(), to_imgui_size(size)), on_click);
	}

	trinex_implement_ui_element(GhostButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(text_align);
		trinex_ui_bind_property(background_color);
	}

	GhostButton& GhostButton::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_ButtonTextAlign, text_align);
		push_style_color(ImGuiCol_Button, background_color);
		push_style_color(ImGuiCol_ButtonHovered, background_color);
		push_style_color(ImGuiCol_ButtonActive, background_color);
		return *this;
	}

	GhostButton& GhostButton::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags GhostButton::on_begin_update()
	{
		return handle_click(this, ImGui::Button(label.c_str(), to_imgui_size(size)), on_click);
	}

	trinex_implement_ui_element(DangerButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(text_align);
		trinex_ui_bind_property(background_color);
	}

	DangerButton& DangerButton::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_ButtonTextAlign, text_align);
		push_style_color(ImGuiCol_Button, background_color);
		push_style_color(ImGuiCol_ButtonHovered, background_color);
		push_style_color(ImGuiCol_ButtonActive, background_color);
		return *this;
	}

	DangerButton& DangerButton::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags DangerButton::on_begin_update()
	{
		const bool clicked = ImGui::Button(label.c_str(), to_imgui_size(size));
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(InvisibleButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags InvisibleButton::on_begin_update()
	{
		return handle_click(this, ImGui::InvisibleButton(label.c_str(), to_imgui_size(size)), on_click);
	}

	trinex_implement_ui_element(RadioButton)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("option", &This::option);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(check_color);
	}

	RadioButton& RadioButton::push_style()
	{
		Super::push_style();
		push_style_color(ImGuiCol_CheckMark, check_color);
		return *this;
	}

	RadioButton& RadioButton::pop_style()
	{
		ImGui::PopStyleColor();
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags RadioButton::on_begin_update()
	{
		return handle_click(this, ImGui::RadioButton(label.c_str(), &value, option), on_click);
	}

	trinex_implement_ui_element(Selectable)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(rounding);
		trinex_ui_bind_property(text_align);
		trinex_ui_bind_property(background_color);
	}

	Selectable& Selectable::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_SelectableRounding, rounding);
		push_style_var(ImGuiStyleVar_SelectableTextAlign, text_align);
		push_style_color(ImGuiCol_Header, background_color);
		push_style_color(ImGuiCol_HeaderHovered, background_color);
		push_style_color(ImGuiCol_HeaderActive, background_color);
		return *this;
	}

	Selectable& Selectable::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags Selectable::on_begin_update()
	{
		return handle_click(this, ImGui::Selectable(label.c_str(), selected, 0, to_imgui_size(size)), on_click);
	}

	trinex_implement_ui_element(ProgressBar)
	{
		reflection()->bind("value", &This::value);
		reflection()->bind("size", &This::size);
		reflection()->bind("overlay", &This::overlay);
		trinex_ui_bind_property(color);
	}

	ProgressBar& ProgressBar::push_style()
	{
		Super::push_style();
		push_style_color(ImGuiCol_PlotHistogram, color);
		push_style_color(ImGuiCol_PlotHistogramHovered, color);
		return *this;
	}

	ProgressBar& ProgressBar::pop_style()
	{
		ImGui::PopStyleColor(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags ProgressBar::on_begin_update()
	{
		ImGui::ProgressBar(value, to_imgui_size(size), overlay.empty() ? nullptr : overlay.c_str());
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
		ImGui::PushID(this);
		ImGui::TextUnformatted(id.empty() ? "..." : id.c_str());
		ImGui::PopID();
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
		int flags = alpha ? 0 : ImGuiColorEditFlags_NoAlpha;
		return item_state_flags(readback_if(ImGui::ColorEdit4(label.c_str(), &color.x, flags)));
	}
}// namespace Trinex::UI
