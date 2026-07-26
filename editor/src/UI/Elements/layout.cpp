#include <UI/Elements/layout.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>
#include <imgui_stacklayout.h>

namespace Trinex::UI
{
	static ImVec2 to_imgui_size(Size size)
	{
		return ImVec2(size.width.value, size.height.value);
	}

	trinex_implement_ui_element(Panel)
	{
		reflection()->bind("id", &This::id);
		reflection()->bind("size", &This::size);
		reflection()->bind("border", &This::border);
		reflection()->bind("background", &This::background);
		trinex_ui_bind_property(padding);
		trinex_ui_bind_property(rounding);
		trinex_ui_bind_property(border_size);
		trinex_ui_bind_property(background_color);
	}

	Panel& Panel::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_WindowPadding, padding);
		push_style_var(ImGuiStyleVar_ChildRounding, rounding);
		push_style_var(ImGuiStyleVar_ChildBorderSize, border_size);
		push_style_color(ImGuiCol_ChildBg, background_color);
		return *this;
	}

	Panel& Panel::pop_style()
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags Panel::on_begin_update()
	{
		ImGui::PushID(this);
		const bool visible = ImGui::BeginChild(id.empty() ? "##Panel" : id.c_str(), to_imgui_size(size), border);
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
		trinex_ui_bind_property(padding);
		trinex_ui_bind_property(rounding);
		trinex_ui_bind_property(border_size);
		trinex_ui_bind_property(background_color);
	}

	GroupPanel& GroupPanel::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_WindowPadding, padding);
		push_style_var(ImGuiStyleVar_ChildRounding, rounding);
		push_style_var(ImGuiStyleVar_ChildBorderSize, border_size);
		push_style_color(ImGuiCol_ChildBg, background_color);
		return *this;
	}

	GroupPanel& GroupPanel::pop_style()
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags GroupPanel::on_begin_update()
	{
		ImGui::PushID(this);
		ImGui::TextUnformatted(label.c_str());
		const bool visible = ImGui::BeginChild("##GroupPanel", to_imgui_size(size), border);
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
		reflection()->bind("id", &This::id);
		reflection()->bind("size", &This::size);
		reflection()->bind("align", &This::align);
	}

	Element::UpdateFlags Horizontal::on_begin_update()
	{
		ImGui::BeginHorizontal(this, size, align);
		return UpdateFlags::Childs | UpdateFlags::End;
	}

	Element& Horizontal::on_end_update(UpdateFlags flags)
	{
		ImGui::EndHorizontal();
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
		ImGui::BeginVertical(this, size, align);
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
		reflection()->bind("id", &This::id);
		reflection()->bind("size", &This::size);
		reflection()->bind("border", &This::border);
		trinex_ui_bind_property(padding);
		trinex_ui_bind_property(rounding);
		trinex_ui_bind_property(border_size);
		trinex_ui_bind_property(background_color);
		trinex_ui_bind_property(scrollbar_bg_color);
		trinex_ui_bind_property(scrollbar_grab_color);
	}

	ScrollArea& ScrollArea::push_style()
	{
		Super::push_style();
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

	ScrollArea& ScrollArea::pop_style()
	{
		ImGui::PopStyleColor(5);
		ImGui::PopStyleVar(3);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags ScrollArea::on_begin_update()
	{
		ImGui::PushID(this);
		const bool visible = ImGui::BeginChild(id.empty() ? "##ScrollArea" : id.c_str(), to_imgui_size(size), border);
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
		reflection()->bind("id", &This::id);
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
		trinex_ui_bind_property(size);
		trinex_ui_bind_property(text_border);
		trinex_ui_bind_property(text_align);
		trinex_ui_bind_property(text_padding);
		trinex_ui_bind_property(color);
	}

	Separator& Separator::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_SeparatorSize, size);
		push_style_var(ImGuiStyleVar_SeparatorTextBorderSize, text_border);
		push_style_var(ImGuiStyleVar_SeparatorTextAlign, text_align);
		push_style_var(ImGuiStyleVar_SeparatorTextPadding, text_padding);
		push_style_color(ImGuiCol_Separator, color);
		push_style_color(ImGuiCol_SeparatorHovered, color);
		push_style_color(ImGuiCol_SeparatorActive, color);
		return *this;
	}

	Separator& Separator::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(4);
		return *Super::pop_style().as<This>();
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
		ImGui::Dummy(ImVec2(0.0f, amount.value));
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
		reflection()->bind("spacing", &This::spacing);
	}

	Element::UpdateFlags SameLine::on_begin_update()
	{
		ImGui::SameLine(offset, spacing);
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Indent)
	{
		reflection()->bind("amount", &This::amount);
	}

	Element::UpdateFlags Indent::on_begin_update()
	{
		ImGui::Indent(amount.value);
		return UpdateFlags::Default;
	}

	Element& Indent::on_end_update(UpdateFlags flags)
	{
		ImGui::Unindent(amount.value);
		return *this;
	}

	trinex_implement_ui_element(Dummy)
	{
		reflection()->bind("size", &This::size);
	}

	Element::UpdateFlags Dummy::on_begin_update()
	{
		ImGui::Dummy(to_imgui_size(size));
		return UpdateFlags::Undefined;
	}

	trinex_implement_ui_element(Spring)
	{
		reflection()->bind("weight", &This::weight);
		reflection()->bind("spacing", &This::spacing);
	}

	Element::UpdateFlags Spring::on_begin_update()
	{
		ImGui::Dummy(ImVec2(spacing, 0.0f));
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
