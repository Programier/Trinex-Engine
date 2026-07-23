#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Window : public Element
	{
		trinex_ui_element(Window, Element);

	public:
		String title;

	public:
		UpdateFlags on_begin_update() override;
		Element& on_end_update() override;
	};
}// namespace Trinex::UI
