#include <UI/Elements/window.hpp>
#include <UI/api.hpp>
#include <UI/element_registry.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Window)
	{
		reflection()->bind<&Window::name>("name");
	}

	bool Window::on_begin_render()
	{
		if (name.empty())
			return false;

		return UI::begin_window(name);
	}

	Window& Window::on_end_render()
	{
		UI::end_window();
		return *this;
	}
}// namespace Trinex::UI
