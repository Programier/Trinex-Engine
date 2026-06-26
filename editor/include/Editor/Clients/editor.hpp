#pragma once
#include <UI/client.hpp>

namespace Trinex
{
	namespace UI
	{
		class Widget;
	}

	class ContentBrowser;

	class EditorClient : public UI::Client
	{
		trinex_class(EditorClient, Client);

	private:
		UI::Widget* m_viewport            = nullptr;
		ContentBrowser* m_content_browser = nullptr;


	private:
		EditorClient& on_render_viewport();

	public:
		EditorClient& attach(class RenderViewport* viewport) override;
		EditorClient& deattach(class RenderViewport* viewport) override;

		EditorClient& setup_dockspace(UI::DockLayout& layout) override;
		EditorClient& select(Object* object) override;
		EditorClient& update(float dt) override;
	};
}// namespace Trinex
