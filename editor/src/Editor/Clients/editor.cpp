#include <Core/reflection/class.hpp>
#include <Editor/Clients/editor.hpp>
#include <UI/Elements/document.hpp>
#include <imgui.h>

namespace Trinex
{
	trinex_implement_class(Trinex::EditorClient, 0) {}

	EditorClient& EditorClient::on_render_viewport()
	{
		return *this;
	}

	EditorClient& EditorClient::attach(class RenderViewport* viewport)
	{
		Super::attach(viewport);
		if (auto doc = load_document("[ui]:/TrinexEditor/clients/editor.ui"))
		{
			doc->open();
		}
		return *this;
	}

	EditorClient& EditorClient::deattach(class RenderViewport* viewport)
	{
		Super::deattach(viewport);
		return *this;
	}

	// EditorClient& EditorClient::setup_dockspace(UI::DockLayout& layout)
	// {
	// 	Super::setup_dockspace(layout);
	// 	return *this;
	// }

	EditorClient& EditorClient::select(Object* object)
	{
		Super::select(object);
		return *this;
	}

	EditorClient& EditorClient::update(float dt)
	{
		Super::update(dt);
		return *this;
	}
}// namespace Trinex
