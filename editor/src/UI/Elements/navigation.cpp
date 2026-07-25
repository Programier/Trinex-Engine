#include <UI/Elements/navigation.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	static Element::UpdateFlags handle_click(Element* element, bool clicked, const Name& event)
	{
		if (clicked)
		{
			element->dispatch(event);
		}

		return Element::item_state_flags(Element::readback_if(clicked));
	}

	static ImVec2 to_imgui_size(Size size)
	{
		return ImVec2(size.width.value, size.height.value);
	}

	trinex_implement_ui_element(CollapsingHeader)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("right_text", &This::right_text);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("disabled", &This::disabled);
	}

	Element::UpdateFlags CollapsingHeader::on_begin_update()
	{
		ImGuiTreeNodeFlags flags = default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		if (ImGui::CollapsingHeader(label.c_str(), flags))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::Undefined;
	}

	Element& CollapsingHeader::on_end_update(UpdateFlags flags)
	{
		return *this;
	}

	trinex_implement_ui_element(SectionHeader)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("right_text", &This::right_text);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("disabled", &This::disabled);
	}

	Element::UpdateFlags SectionHeader::on_begin_update()
	{
		ImGuiTreeNodeFlags flags = default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		if (ImGui::CollapsingHeader(label.c_str(), flags))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::Undefined;
	}

	Element& SectionHeader::on_end_update(UpdateFlags flags)
	{
		return *this;
	}

	trinex_implement_ui_element(TreeNode)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("selected", &This::selected);
	}

	Element::UpdateFlags TreeNode::on_begin_update()
	{
		ImGuiTreeNodeFlags flags = default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		if (selected)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (ImGui::TreeNodeEx(label.c_str(), flags))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::End;
	}

	Element& TreeNode::on_end_update(UpdateFlags flags)
	{
		ImGui::TreePop();
		return *this;
	}

	trinex_implement_ui_element(TreeLeaf)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags TreeLeaf::on_begin_update()
	{
		return handle_click(this, ImGui::Selectable(label.c_str(), selected), on_click);
	}

	trinex_implement_ui_element(TabBar)
	{
		reflection()->bind("id", &This::id);
	}

	Element::UpdateFlags TabBar::on_begin_update()
	{
		ImGui::PushID(this);
		if (ImGui::BeginTabBar(id.empty() ? "##TabBar" : id.c_str()))
		{
			return UpdateFlags::Default;
		}

		ImGui::PopID();
		return UpdateFlags::Undefined;
	}

	Element& TabBar::on_end_update(UpdateFlags flags)
	{
		ImGui::EndTabBar();
		ImGui::PopID();
		return *this;
	}

	trinex_implement_ui_element(Tab)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags Tab::on_begin_update()
	{
		bool open = true;
		const bool visible = ImGui::BeginTabItem(label.c_str(), &open);
		const bool clicked = ImGui::IsItemClicked();
		if (visible)
		{
			ImGui::EndTabItem();
		}
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(SidebarItem)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags SidebarItem::on_begin_update()
	{
		return handle_click(this, ImGui::Selectable(label.c_str(), selected), on_click);
	}

	trinex_implement_ui_element(NavItem)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags NavItem::on_begin_update()
	{
		return handle_click(this, ImGui::Selectable(label.c_str(), selected), on_click);
	}

	trinex_implement_ui_element(Breadcrumb)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("current", &This::current);
		reflection()->bind("on_click", &This::on_click);
	}

	Element::UpdateFlags Breadcrumb::on_begin_update()
	{
		if (!current)
		{
			ImGui::TextUnformatted("/");
			ImGui::SameLine();
		}
		return handle_click(this, ImGui::Selectable(label.c_str(), current, 0, to_imgui_size(Size(0.0f, 0.0f))), on_click);
	}
}// namespace Trinex::UI
