#pragma once
#include <UI/Elements/visual.hpp>

namespace Trinex::UI
{
	class FramedGeometry : public Visual
	{
		trinex_ui_element(FramedGeometry, Visual);

	public:
		ImVec2 padding  = {4.0f, 3.0f};
		f32 rounding    = 0.0f;
		f32 border_size = 0.0f;
		
		FramedGeometry& push_style() override;
		FramedGeometry& pop_style() override;
	};

	class Framed : public Visual
	{
		trinex_ui_element(Framed, Visual);

	public:
		ImVec4 color = {0.20f, 0.20f, 0.25f, 0.94f};

		Framed& push_style() override;
		Framed& pop_style() override;
	};
}// namespace Trinex::UI
