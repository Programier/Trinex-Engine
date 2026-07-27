#include <UI/Elements/docking.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace Trinex::UI
{
	static bool is_valid_dock_size(const ImVec2& size)
	{
		return size.x > 0.0f && size.y > 0.0f;
	}

	static ImGuiDir to_imgui_dir(Name dir)
	{
		static Name left   = "left";
		static Name up     = "up";
		static Name top    = "top";
		static Name down   = "down";
		static Name bottom = "bottom";

		if (dir == left)
			return ImGuiDir_Left;
		if (dir == up || dir == top)
			return ImGuiDir_Up;
		if (dir == down || dir == bottom)
			return ImGuiDir_Down;

		return ImGuiDir_Right;
	}

	static bool build_dock_child(DockSpace* space, Element* element)
	{
		bool result = true;

		if (element->type()->is_a<DockSplit>())
		{
			result &= element->as<DockSplit>()->build(space);
		}

		if (element->type()->is_a<DockWindow>())
		{
			result &= element->as<DockWindow>()->build(space);
		}

		for (Element* child : element->childs())
		{
			result &= build_dock_child(space, child);
		}

		return result;
	}

	trinex_implement_ui_element(DockSpace)
	{
		reflection()->bind("size", &This::size);
		reflection()->bind("flags", &This::flags);
		reflection()->bind("rebuild", &This::rebuild);
	}

	DockSpace& DockSpace::bind_dock(Name name, u32 dock)
	{
		if (name.is_valid() && dock != 0)
		{
			m_docks[name] = dock;
		}

		return *this;
	}

	u32 DockSpace::find_dock(Name name) const
	{
		auto it = m_docks.find(name);
		return it == m_docks.end() ? 0 : it->second;
	}

	u32 DockSpace::require_dock(Name name) const
	{
		return name.is_valid() ? find_dock(name) : find_dock("main");
	}

	DockSpace& DockSpace::build_layout(ImVec2 size)
	{
		const u32 root = require_dock("main");
		if (root == 0 || !is_valid_dock_size(size))
		{
			return *this;
		}

		ImGui::DockBuilderRemoveNode(root);
		ImGui::DockBuilderAddNode(root, static_cast<ImGuiDockNodeFlags>(flags) | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(root, size);

		bind_dock("main", root);
		for (Element* child : childs())
		{
			build_dock_child(this, child);
		}

		ImGui::DockBuilderFinish(root);
		m_built = true;
		rebuild = false;
		return *this;
	}

	Element::UpdateFlags DockSpace::on_begin_update()
	{
		UpdateFlags result = Super::on_begin_update();
		if (!(result & UpdateFlags::Childs))
		{
			return result;
		}

		const char* name = id().is_valid() ? id().c_str() : "##DockSpace";
		const u32 root   = ImGui::GetID(name);

		bind_dock("main", root);
		if (id().is_valid())
		{
			bind_dock(id(), root);
		}

		ImVec2 dock_size = resolve(size);
		if (dock_size.x == 0.0f && dock_size.y == 0.0f)
		{
			dock_size = ImGui::GetContentRegionAvail();
		}

		ImGui::DockSpace(root, dock_size, static_cast<ImGuiDockNodeFlags>(flags));

		if (!m_built || rebuild)
		{
			build_layout(dock_size);
		}

		return result;
	}

	Element& DockSpace::on_end_update(UpdateFlags flags)
	{
		return Super::on_end_update(flags);
	}

	trinex_implement_ui_element(DockSplit)
	{
		reflection()->bind("from", &This::from);
		reflection()->bind("dir", &This::dir);
		reflection()->bind("ratio", &This::ratio);
		reflection()->bind("child", &This::child);
		reflection()->bind("remainder", &This::remainder);
	}

	Element::UpdateFlags DockSplit::on_begin_update()
	{
		return UpdateFlags::Undefined;
	}

	bool DockSplit::build(DockSpace* space)
	{
		if (space == nullptr)
		{
			return false;
		}

		if (!from.is_valid())
		{
			Element* element = owner();

			while (element)
			{
				if (element->type()->is_a<DockSplit>())
				{
					auto super = element->as<DockSplit>();
					if (super->remainder.is_valid())
					{
						from = super->remainder;
						break;
					}
				}

				element = element->owner();
			}
		}

		const u32 source = space->require_dock(from);

		if (source == 0)
		{
			return false;
		}

		u32 child_dock     = 0;
		u32 remainder_dock = 0;
		ImGui::DockBuilderSplitNode(source, to_imgui_dir(dir), ratio, &child_dock, &remainder_dock);
		space->bind_dock(from.is_valid() ? from : Name("main"), remainder_dock);
		space->bind_dock(child, child_dock);
		space->bind_dock(remainder, remainder_dock);
		return true;
	}

	trinex_implement_ui_element(DockWindow)
	{
		reflection()->bind("window", &This::window);
		reflection()->bind("dock", &This::dock);
	}

	Element::UpdateFlags DockWindow::on_begin_update()
	{
		return UpdateFlags::Undefined;
	}

	bool DockWindow::build(DockSpace* space)
	{
		if (space == nullptr || window.empty())
		{
			return false;
		}

		const u32 dock_id = space->require_dock(dock);
		if (dock_id == 0)
		{
			return false;
		}

		ImGui::DockBuilderDockWindow(window.c_str(), dock_id);
		return true;
	}
}// namespace Trinex::UI
