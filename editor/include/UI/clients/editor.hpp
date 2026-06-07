#pragma once
#include <UI/controllers/canvas.hpp>

namespace Trinex::UI
{
	class RMLEditor : public RMLClient
	{
		trinex_class(RMLEditor, RMLClient);

	private:
		RML::ElementDocument* m_document = nullptr;

	public:
		RMLEditor& attach(class RenderViewport* viewport) override;
		RMLEditor& deattach(class RenderViewport* viewport) override;
		RMLEditor& update(class RenderViewport* viewport, float dt) override;
		RMLCanvasFrame render(RML::Element* viewport, const RMLCanvasRenderArgs& args);
	};
}// namespace Trinex::UI
