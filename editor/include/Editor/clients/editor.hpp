#pragma once
#include <UI/client.hpp>

namespace Trinex
{
	class EditorClient : public UI::Client
	{
		trinex_class(EditorClient, Client);

	public:
		EditorClient& attach(class RenderViewport* viewport) override;
		EditorClient& deattach(class RenderViewport* viewport) override;

		EditorClient& setup_dockspace(UI::DockLayoutBuilder& builder) override;
		EditorClient& select(Object* object) override;
		EditorClient& update(float dt) override;
	};
}// namespace Trinex
