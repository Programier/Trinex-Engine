#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Visual : public Element
	{
		trinex_ui_element(Visual, Element);

	public:
		ImVec2 pivot     = {0.5, 0.5};
		ImVec2 translate = {0.f, 0.f};
		ImVec2 scale     = {1.f, 1.f};
		f32 rotate       = 0.f;

		Visual& push_style() override;
		Visual& pop_style() override;
	};
}// namespace Trinex::UI
