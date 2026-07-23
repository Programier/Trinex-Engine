#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Label : public Element
	{
		trinex_ui_element(Label, Element);

	public:
		String label;
		String value;

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
