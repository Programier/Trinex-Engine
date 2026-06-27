#include <Core/reflection/class.hpp>
#include <Editor/Clients/editor.hpp>
#include <Editor/Widgets/console.hpp>
#include <Editor/Widgets/content_browser.hpp>
#include <UI/api.hpp>

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

		m_viewport        = UI::Widget::create("Viewport###viewport", true, [this]() { on_render_viewport(); });
		m_content_browser = trx_new ContentBrowserWidget();
		m_console         = trx_new ConsoleWidget({}, true);

		m_content_browser->is_open(true);

		UI::register_widget(context(), m_viewport);
		UI::register_widget(context(), m_content_browser);
		UI::register_widget(context(), m_console);
		return *this;
	}

	EditorClient& EditorClient::deattach(class RenderViewport* viewport)
	{
		UI::unregister_widget(context(), m_console);
		UI::unregister_widget(context(), m_viewport);
		UI::unregister_widget(context(), m_content_browser);

		trx_delete m_console;
		trx_delete m_content_browser;
		trx_delete m_viewport;

		Super::deattach(viewport);
		return *this;
	}

	EditorClient& EditorClient::setup_dockspace(UI::DockLayout& layout)
	{
		Super::setup_dockspace(layout);
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

		if (UI::key_ctrl() && UI::is_key_pressed(UI::Key::P, false))
			UI::open_command_palette();

		UI::command_palette();
		return *this;
	}
}// namespace Trinex
