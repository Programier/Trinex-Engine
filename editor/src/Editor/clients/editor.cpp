#include <Core/reflection/class.hpp>
#include <Editor//clients/editor.hpp>
#include <UI/api.hpp>

namespace Trinex
{
	trinex_implement_class(Trinex::EditorClient, 0) {}

	EditorClient& EditorClient::attach(class RenderViewport* viewport)
	{
		Super::attach(viewport);
		return *this;
	}

	EditorClient& EditorClient::deattach(class RenderViewport* viewport)
	{
		Super::deattach(viewport);
		return *this;
	}

	EditorClient& EditorClient::setup_dockspace(UI::DockLayoutBuilder& builder)
	{
		Super::setup_dockspace(builder);
		return *this;
	}

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
