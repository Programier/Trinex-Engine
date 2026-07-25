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
	}

	Element::UpdateFlags DangerButton::on_begin_update()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.16f, 0.16f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.08f, 0.08f, 1.0f));
		const bool clicked = ImGui::Button(label.c_str(), to_imgui_size(size));
		ImGui::PopStyleColor(3);
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
