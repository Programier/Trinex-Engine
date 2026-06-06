#pragma once
#include <UI/rml.hpp>

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
	};
}// namespace Trinex::UI
