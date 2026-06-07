#pragma once
#include <RmlUi/Core/Element.h>

namespace Trinex::UI
{
	namespace RML = Rml;

	class RMLCanvasElement : public RML::Element
	{
	public:
		RMLCanvasElement(const RML::String& tag) : Rml::Element(tag) {}
		void OnRender() override;
	};
}// namespace Trinex::UI
