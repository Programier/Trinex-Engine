#pragma once
#include <UI/Elements/framed.hpp>

namespace Trinex::UI
{
	class Button : public FramedGeometry
	{
		trinex_ui_element(Button, FramedGeometry);

	public:
		String label;
		Size size = Size(0.0f, 0.0f);
		Name on_click;
		ImVec2 text_align = {0.5f, 0.5f};
		ImVec4 color      = {0.20f, 0.25f, 0.30f, 0.94f};

		Button& push_scope() override;
		Button& pop_scope() override;
		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
