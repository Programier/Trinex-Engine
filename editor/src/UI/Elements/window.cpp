#include <UI/Elements/window.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Window)
	{
		reflection()->bind("title", &Window::title);
	}

	Element::UpdateFlags Window::on_begin_update()
	{
		if (title.empty())
			return UpdateFlags::Undefined;

		if (ImGui::Begin(title.c_str()))
			return UpdateFlags::Default;

		return UpdateFlags::End;
	}

	Element& Window::on_end_update()
	{
		ImGui::End();
		return *this;
	}
}// namespace Trinex::UI
