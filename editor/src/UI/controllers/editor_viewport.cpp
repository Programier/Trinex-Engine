#include <Core/reflection/class.hpp>
#include <UI/clients/editor.hpp>
#include <UI/controllers/editor_viewport.hpp>

namespace Trinex::UI
{
	trinex_implement_class(Trinex::UI::EditorViewportController, 0) {}

	RMLCanvasFrame EditorViewportController::render(RML::Element* element, const RMLCanvasRenderArgs& args) const
	{
		if (auto editor = Object::instance_cast<RMLEditor>(owner()))
		{
			return editor->render(element, args);
		}

		return {};
	}
}// namespace Trinex::UI
