#include <UI/Elements/layout.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

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
		ImGui::BeginGroup();
		return UpdateFlags::Default;
	}

	Element& Horizontal::on_end_update(UpdateFlags flags)
	{
		ImGui::EndGroup();
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
		ImGui::BeginGroup();
		return UpdateFlags::Default;
	}

	Element& Vertical::on_end_update(UpdateFlags flags)
	{
		ImGui::EndGroup();
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

	trinex_implement_ui_element(Separator) {}

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
