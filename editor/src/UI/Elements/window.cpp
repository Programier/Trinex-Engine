#include <UI/Elements/window.hpp>
#include <UI/api.hpp>
#include <UI/element_registry.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Window)
	{
		reflection()->property<&Window::name>("name");
	}

	bool Window::on_begin_render()
	{
		return UI::begin_window(name);
	}

	Window& Window::on_end_render()
	{
		UI::end_window();
		return *this;
	}
}// namespace Trinex::UI
