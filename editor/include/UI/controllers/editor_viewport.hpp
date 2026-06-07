#pragma once
#include <UI/controllers/canvas.hpp>

namespace Trinex::UI
{
	class EditorViewportController final : public RMLCanvasController
	{
		trinex_class(EditorViewportController, RMLCanvasController);

	public:
		RMLCanvasFrame render(RML::Element* element, const RMLCanvasRenderArgs& args) const override;
	};
}// namespace Trinex::UI
