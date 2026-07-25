#pragma once
#include <UI/client.hpp>

namespace Trinex
{
	namespace UI
	{
		class Document;
		class Widget;
	}// namespace UI

	class ContentBrowserWidget;
	class ConsoleWidget;

	class EditorClient : public UI::Client
	{
		trinex_class(EditorClient, Client);

	private:
		UI::Widget* m_viewport                  = nullptr;
		UI::Document* m_document                = nullptr;
		ContentBrowserWidget* m_content_browser = nullptr;
		ConsoleWidget* m_console                = nullptr;


	private:
		EditorClient& on_render_viewport();

	public:
		EditorClient& attach(class RenderViewport* viewport) override;
		EditorClient& deattach(class RenderViewport* viewport) override;

		//EditorClient& setup_dockspace(UI::DockLayout& layout) override;
		EditorClient& select(Object* object) override;
		EditorClient& update(float dt) override;
	};
}// namespace Trinex
