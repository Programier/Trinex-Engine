#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Window : public Element
	{
		trinex_ui_element(Window, Element);

	public:
		String name;

	public:
		bool on_begin_render() override;
		Window& on_end_render() override;
	};
}// namespace Trinex::UI
