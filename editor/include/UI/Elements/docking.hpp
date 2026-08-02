#pragma once
#include <Core/etl/flat_map.hpp>
#include <UI/Elements/window.hpp>

struct ImVec2;

namespace Trinex::UI
{
	class DockSpace : public Window
	{
		trinex_ui_element(DockSpace, Window);

	protected:
		FlatMap<Name, u32> m_docks;

		DockSpace& bind_dock(Name name, u32 dock);
		u32 find_dock(Name name) const;
		u32 require_dock(Name name) const;
		DockSpace& build_layout(ImVec2 size);

	public:
		Size size                        = Size(0.0f, 0.0f);
		ImGuiDockNodeFlags docking_flags = 0;

		UpdateFlags on_begin_update() override;

		friend class DockSplit;
		friend class DockWindow;
	};

	class DockSpaceOverViewport : public DockSpace
	{
		trinex_ui_element(DockSpaceOverViewport, DockSpace);

	public:
		UpdateFlags on_begin_update() override;
	};

	class DockSplit : public Element
	{
		trinex_ui_element(DockSplit, Element);

	public:
		Name from;
		Name dir  = "right";
		f32 ratio = 0.5f;
		Name child;
		Name remainder;

		UpdateFlags on_begin_update() override;
		bool build(DockSpace* space);
	};

	class DockWindow : public Element
	{
		trinex_ui_element(DockWindow, Element);

	public:
		String window;
		Name dock;

		UpdateFlags on_begin_update() override;
		bool build(DockSpace* space);
	};
}// namespace Trinex::UI
