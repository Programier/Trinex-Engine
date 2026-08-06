#include <UI/Elements/layout.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>
#include <imgui_stacklayout.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Panel)
	{
		reflection()->bind("size", &This::size);
		reflection()->bind("border", &This::border);
		reflection()->bind("background", &This::background);
		trinex_ui_bind_property(padding, Style);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(border_size, Style);
		trinex_ui_bind_property(background_color, Style);
	}

	Panel& Panel::push_scope()
	{
		Super::push_scope();
		push_style_var(ImGuiStyleVar_WindowPadding, padding);
		push_style_var(ImGuiStyleVar_ChildRounding, rounding);
		push_style_var(ImGuiStyleVar_ChildBorderSize, border_size);
		push_style_color(ImGuiCol_ChildBg, background_color);
		return *this;
	}

	Panel& Panel::pop_scope()
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags Panel::on_begin_update()
	{
		ImGui::PushID(this);
		const bool visible = ImGui::BeginChild(id().is_valid() ? id().c_str() : "##Panel", resolve(size), border);
		if (!visible)
		{
			ImGui::EndChild();
			ImGui::PopID();
			return UpdateFlags::Undefined;
		}
		return UpdateFlags::Default;
	}

	Element& Panel::on_end_update(UpdateFlags flags)
	{
		ImGui::EndChild();
		ImGui::PopID();
		return *this;
	}

	trinex_implement_ui_element(GroupPanel)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("size", &This::size);
		reflection()->bind("border", &This::border);
		reflection()->bind("background", &This::background);
		trinex_ui_bind_property(padding, Style);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(border_size, Style);
		trinex_ui_bind_property(background_color, Style);
	}

	GroupPanel& GroupPanel::push_scope()
	{
		Super::push_scope();
		push_style_var(ImGuiStyleVar_WindowPadding, padding);
		push_style_var(ImGuiStyleVar_ChildRounding, rounding);
		push_style_var(ImGuiStyleVar_ChildBorderSize, border_size);
		push_style_color(ImGuiCol_ChildBg, background_color);
		return *this;
	}

	GroupPanel& GroupPanel::pop_scope()
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags GroupPanel::on_begin_update()
	{
		ImGui::PushID(this);
		ImGui::TextUnformatted(label.c_str());
		const bool visible = ImGui::BeginChild("##GroupPanel", resolve(size), border);
		if (!visible)
		{
			ImGui::EndChild();
			ImGui::PopID();
			return UpdateFlags::Undefined;
		}
		return UpdateFlags::Default;
	}

	Element& GroupPanel::on_end_update(UpdateFlags flags)
	{
		ImGui::EndChild();
		ImGui::PopID();
		return *this;
	}

	trinex_implement_ui_element(Group) {}

	Element::UpdateFlags Group::on_begin_update()
	{
		ImGui::BeginGroup();
		return UpdateFlags::Default;
	}

	Element& Group::on_end_update(UpdateFlags flags)
	{
		ImGui::EndGroup();
		return *this;
	}

	trinex_implement_ui_element(Horizontal)
	{
		reflection()->bind("size", &This::size);
		reflection()->bind("align", &This::align);
	}

	Element::UpdateFlags Horizontal::on_begin_update()
	{
		ImGui::BeginHorizontal(this, resolve(size), align);
		return UpdateFlags::Childs | UpdateFlags::End;
	}

	Element& Horizontal::on_end_update(UpdateFlags flags)
	{
		ImGui::EndHorizontal();
		return *this;
	}

	trinex_implement_ui_element(Vertical)
	{
		reflection()->bind("size", &This::size);
		reflection()->bind("align", &This::align);
	}

	Element::UpdateFlags Vertical::on_begin_update()
	{
		ImGui::BeginVertical(this, resolve(size), align);
		return UpdateFlags::Childs | UpdateFlags::End;
	}

	Element& Vertical::on_end_update(UpdateFlags flags)
	{
		ImGui::EndVertical();
		return *this;
	}

	trinex_implement_ui_element(Disabled)
	{
		reflection()->bind("disabled", &This::disabled);
	}

	Element::UpdateFlags Disabled::on_begin_update()
	{
		ImGui::BeginDisabled(disabled);
		return UpdateFlags::Default;
	}

	Element& Disabled::on_end_update(UpdateFlags flags)
	{
		ImGui::EndDisabled();
		return *this;
	}

	trinex_implement_ui_element(ScrollArea)
	{
		reflection()->bind("size", &This::size);
		reflection()->bind("border", &This::border);
		trinex_ui_bind_property(padding, Style);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(border_size, Style);
		trinex_ui_bind_property(background_color, Style);
		trinex_ui_bind_property(scrollbar_bg_color, Style);
		trinex_ui_bind_property(scrollbar_grab_color, Style);
	}

	ScrollArea& ScrollArea::push_scope()
	{
		Super::push_scope();
		push_style_var(ImGuiStyleVar_WindowPadding, padding);
		push_style_var(ImGuiStyleVar_ChildRounding, rounding);
		push_style_var(ImGuiStyleVar_ChildBorderSize, border_size);
		push_style_color(ImGuiCol_ChildBg, background_color);
		push_style_color(ImGuiCol_ScrollbarBg, scrollbar_bg_color);
		push_style_color(ImGuiCol_ScrollbarGrab, scrollbar_grab_color);
		push_style_color(ImGuiCol_ScrollbarGrabHovered, scrollbar_grab_color);
		push_style_color(ImGuiCol_ScrollbarGrabActive, scrollbar_grab_color);
		return *this;
	}

	ScrollArea& ScrollArea::pop_scope()
	{
		ImGui::PopStyleColor(5);
		ImGui::PopStyleVar(3);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags ScrollArea::on_begin_update()
	{
		ImGui::PushID(this);
		const bool visible = ImGui::BeginChild(id().is_valid() ? id().c_str() : "##ScrollArea", resolve(size), border);
		if (!visible)
		{
			ImGui::EndChild();
			ImGui::PopID();
			return UpdateFlags::Undefined;
		}
		return UpdateFlags::Default;
	}

	Element& ScrollArea::on_end_update(UpdateFlags flags)
	{
		ImGui::EndChild();
		ImGui::PopID();
		return *this;
	}

	trinex_implement_ui_element(AnimatedArea)
	{
		reflection()->bind("visible", &This::visible);
	}

	Element::UpdateFlags AnimatedArea::on_begin_update()
	{
		ImGui::PushID(this);
		const bool opened = visible;
		if (!opened)
		{
			ImGui::PopID();
			return UpdateFlags::Undefined;
		}
		ImGui::BeginGroup();
		return UpdateFlags::Default;
	}

	Element& AnimatedArea::on_end_update(UpdateFlags flags)
	{
		ImGui::EndGroup();
		ImGui::PopID();
		return *this;
	}

	trinex_implement_ui_element(Separator)
	{
		trinex_ui_bind_property(size, Style);
		trinex_ui_bind_property(text_border, Style);
		trinex_ui_bind_property(text_align, Style);
		trinex_ui_bind_property(text_padding, Style);
		trinex_ui_bind_property(color, Style);
	}

	Separator& Separator::push_scope()
	{
		Super::push_scope();
		push_style_var(ImGuiStyleVar_SeparatorSize, size);
		push_style_var(ImGuiStyleVar_SeparatorTextBorderSize, text_border);
		push_style_var(ImGuiStyleVar_SeparatorTextAlign, text_align);
		push_style_var(ImGuiStyleVar_SeparatorTextPadding, text_padding);
		push_style_color(ImGuiCol_Separator, color);
		push_style_color(ImGuiCol_SeparatorHovered, color);
		push_style_color(ImGuiCol_SeparatorActive, color);
		return *this;
	}

	Separator& Separator::pop_scope()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(4);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags Separator::on_begin_update()
	{
		ImGui::Separator();
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Spacing)
	{
		reflection()->bind("amount", &This::amount);
	}

	Element::UpdateFlags Spacing::on_begin_update()
	{
		ImGui::Dummy(ImVec2(0.0f, resolve(amount, Axis::Y)));
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(NewLine) {}

	Element::UpdateFlags NewLine::on_begin_update()
	{
		ImGui::NewLine();
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(SameLine)
	{
		reflection()->bind("offset", &This::offset);
	}

	Element::UpdateFlags SameLine::on_begin_update()
	{
		ImGui::SameLine(resolve(offset, Axis::X));
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Indent)
	{
		reflection()->bind("amount", &This::amount);
	}

	Element::UpdateFlags Indent::on_begin_update()
	{
		ImGui::Indent(resolve(amount, Axis::X));
		return UpdateFlags::Default;
	}

	Element& Indent::on_end_update(UpdateFlags flags)
	{
		ImGui::Unindent(resolve(amount, Axis::X));
		return *this;
	}

	trinex_implement_ui_element(Dummy)
	{
		reflection()->bind("size", &This::size);
	}

	Element::UpdateFlags Dummy::on_begin_update()
	{
		ImGui::Dummy(resolve(size));
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Spring)
	{
		reflection()->bind("weight", &This::weight);
	}

	Element::UpdateFlags Spring::on_begin_update()
	{
		ImGui::Spring(weight);
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
