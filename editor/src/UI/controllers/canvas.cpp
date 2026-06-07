#include <Core/reflection/class.hpp>
#include <UI/controllers/canvas.hpp>

namespace Trinex::UI
{
	trinex_implement_class(Trinex::UI::RMLCanvasController, 0) {}

	RMLCanvasFrame RMLCanvasController::render(RML::Element* element, const RMLCanvasRenderArgs& args) const
	{
		(void) element;
		(void) args;
		return {};
	}
}// namespace Trinex::UI
