#include <UI/Elements/window.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Window)
	{
		reflection()->bind("name", &Window::name);
	}

	Element::UpdateFlags Window::on_begin_update()
	{
		if (name.empty())
			return UpdateFlags::Undefined;

		if (UI::begin_window(name))
			return UpdateFlags::Default;

		return UpdateFlags::End;
	}

	Element& Window::on_end_update()
	{
		UI::end_window();
		return *this;
	}
}// namespace Trinex::UI
