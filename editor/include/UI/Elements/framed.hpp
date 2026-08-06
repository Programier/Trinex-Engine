#pragma once
#include <UI/Elements/visual.hpp>

namespace Trinex::UI
{
	class FramedGeometry : public Visual
	{
		trinex_ui_element(FramedGeometry, Visual);

	public:
		Size padding     = Size(4.0f, 3.0f);
		Unit rounding    = Unit(0.0f);
		Unit border_size = Unit(0.0f);
		
		FramedGeometry& push_scope() override;
		FramedGeometry& pop_scope() override;
	};

	class Framed : public Visual
	{
		trinex_ui_element(Framed, Visual);

	public:
		ImVec4 color = {0.20f, 0.20f, 0.25f, 0.94f};

		Framed& push_scope() override;
		Framed& pop_scope() override;
	};
}// namespace Trinex::UI
