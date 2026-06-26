#include <Core/reflection/class.hpp>
#include <UI/controllers/editor_viewport.hpp>

namespace Trinex::UI
{
	trinex_implement_class(Trinex::UI::EditorViewportController, 0) {}

	RMLCanvasFrame EditorViewportController::render(RML::Element* element, const RMLCanvasRenderArgs& args) const
	{
		return {};
	}
}// namespace Trinex::UI
