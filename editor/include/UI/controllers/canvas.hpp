#pragma once
#include <UI/rml.hpp>

namespace Trinex::UI
{
	enum class RMLCanvasFit : u8
	{
		Stretch,
		Contain,
		Center,
	};

	struct RMLCanvasRenderArgs {
		RML::Element* element = nullptr;
		Vector2f position     = {0.f, 0.f};
		Vector2f size         = {0.f, 0.f};
	};

	struct RMLCanvasFrame {
		RHITexture* texture = nullptr;
		RMLCanvasFit fit    = RMLCanvasFit::Stretch;
	};

	class RMLCanvasController : public RMLController
	{
		trinex_class(RMLCanvasController, RMLController);

	public:
		virtual RMLCanvasFrame render(RML::Element* element, const RMLCanvasRenderArgs& args) const;
	};
}// namespace Trinex::UI
