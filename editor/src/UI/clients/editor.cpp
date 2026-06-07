#include <Core/default_resources.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/texture.hpp>
#include <RmlUi/Core.h>
#include <UI/clients/editor.hpp>

namespace Trinex::UI
{
	trinex_implement_class(Trinex::UI::RMLEditor, 0) {}

	RMLEditor& RMLEditor::attach(class RenderViewport* viewport)
	{
		Super::attach(viewport);

		if ((m_document = context()->LoadDocument("[rml]:/TrinexEditor/layouts/editor/main.rml")))
		{
			m_document->Show();
		}

		return *this;
	}

	RMLEditor& RMLEditor::deattach(class RenderViewport* viewport)
	{
		if (m_document)
		{
			m_document->Close();
			m_document = nullptr;
		}

		Super::deattach(viewport);
		return *this;
	}

	RMLEditor& RMLEditor::update(class RenderViewport* viewport, float dt)
	{
		Super::update(viewport, dt);
		return *this;
	}

	RMLCanvasFrame RMLEditor::render(RML::Element* viewport, const RMLCanvasRenderArgs& args)
	{
		return {.texture = DefaultResources::Textures::default_texture->rhi_texture(), .fit = RMLCanvasFit::Stretch};
	}
}// namespace Trinex::UI
