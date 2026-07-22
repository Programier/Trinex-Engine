#include <UI/Elements/window.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Window)
	{
		reflection()->bind("name", &Window::name);
		reflection()->bind("value", &Window::value);
	}

	bool Window::on_begin_render()
	{
		if (name.empty())
			return false;

		if (UI::begin_window(name))
		{
			UI::text("VALUE: %f\n", value);
			return true;
		}

		return false;
	}

	Window& Window::on_end_render()
	{
		UI::end_window();
		return *this;
	}
}// namespace Trinex::UI
