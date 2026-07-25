// #include <UI/docking.hpp>
// #include <imgui.h>

// namespace Trinex::UI
// {
// 	static ImVec2 resolve_size(Size size)
// 	{
// 		return ImVec2(size.width.value, size.height.value);
// 	}

// 	static ImGuiDir to_imgui_dir(DockSplitDir dir)
// 	{
// 		return static_cast<ImGuiDir>(dir);
// 	}

// 	static ImGuiDockNodeFlags to_imgui_flags(DockNodeFlags flags)
// 	{
// 		return static_cast<ImGuiDockNodeFlags>(flags);
// 	}

// 	bool DockLayout::exists() const
// 	{
// 		return m_root && ImGui::DockBuilderGetNode(m_root.value()) != nullptr;
// 	}

// 	DockLayout& DockLayout::bind(StringView id, DockID dock)
// 	{
// 		if (id.empty())
// 		{
// 			return *this;
// 		}

// 		for (NamedDock& named : m_named)
// 		{
// 			if (named.id == id)
// 			{
// 				named.dock = dock;
// 				return *this;
// 			}
// 		}

// 		m_named.push_back({String(id), dock});
// 		return *this;
// 	}

// 	DockLayout& DockLayout::flags(DockID dock, DockNodeFlags flags)
// 	{
// 		if (dock)
// 		{
// 			ImGui::DockBuilderGetNode(dock.value())->LocalFlags |= to_imgui_flags(flags);
// 		}

// 		return *this;
// 	}

// 	DockLayout& DockLayout::flags(StringView id, DockNodeFlags flags)
// 	{
// 		return this->flags(require(id), flags);
// 	}

// 	DockID DockLayout::find(StringView id) const
// 	{
// 		for (const NamedDock& named : m_named)
// 		{
// 			if (named.id == id)
// 			{
// 				return named.dock;
// 			}
// 		}

// 		return {};
// 	}

// 	DockID DockLayout::require(StringView id) const
// 	{
// 		const DockID result = find(id);
// 		trinex_assert(result && "Required dock id was not found");
// 		return result;
// 	}

// 	bool DockLayout::has(StringView id) const
// 	{
// 		return find(id) != DockID();
// 	}

// 	DockLayout::Result DockLayout::split(DockID dock, DockSplitDir dir, f32 ratio, StringView id)
// 	{
// 		return split(dock, dir, ratio, {}, id);
// 	}

// 	DockLayout::Result DockLayout::split(DockID dock, DockSplitDir dir, f32 ratio, StringView remainder_id, StringView child_id)
// 	{
// 		DockID remainder;
// 		DockID child = ImGui::DockBuilderSplitNode(dock.value(), to_imgui_dir(dir), ratio, &child.value(), &remainder.value());

// 		if (!remainder_id.empty())
// 		{
// 			bind(remainder_id, remainder);
// 		}

// 		if (!child_id.empty())
// 		{
// 			bind(child_id, child);
// 		}

// 		return {.remainder = remainder, .child = child};
// 	}

// 	DockID DockLayout::crop(DockID& dock, DockSplitDir dir, f32 ratio, StringView id)
// 	{
// 		return crop(dock, dir, ratio, {}, id);
// 	}

// 	DockID DockLayout::crop(DockID& dock, DockSplitDir dir, f32 ratio, StringView remainder_id, StringView child_id)
// 	{
// 		Result result = split(dock, dir, ratio, remainder_id, child_id);
// 		dock          = result.remainder;
// 		return result.child;
// 	}

// 	DockID DockLayout::dock(StringView window_name, DockID dock_id)
// 	{
// 		if (dock_id)
// 		{
// 			ImGui::DockBuilderDockWindow(String(window_name).c_str(), dock_id.value());
// 		}

// 		return dock_id;
// 	}

// 	DockID DockLayout::dock(StringView window_name, StringView dock_id)
// 	{
// 		return dock(window_name, require(dock_id));
// 	}

// 	bool DockLayout::begin(Size size, DockNodeFlags flags)
// 	{
// 		return begin(ID::from(this), size, flags);
// 	}

// 	bool DockLayout::begin(DockID root, Size size, DockNodeFlags flags)
// 	{
// 		m_root = root;
// 		m_main = root;

// 		if (!m_root)
// 		{
// 			return false;
// 		}

// 		const ImVec2 resolved = resolve_size(size);
// 		ImGui::DockSpace(m_root.value(), resolved, to_imgui_flags(flags));
// 		return true;
// 	}

// 	DockLayout& DockLayout::end()
// 	{
// 		return *this;
// 	}
// }// namespace Trinex::UI

