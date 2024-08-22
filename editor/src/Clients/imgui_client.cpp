#include <Clients/imgui_client.hpp>
#include <Core/class.hpp>
#include <Core/theme.hpp>
#include <Graphics/imgui.hpp>


namespace Engine
{
	implement_engine_class_default_init(ImGuiEditorClient, 0);

	ImGuiEditorClient& ImGuiEditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		Super::on_bind_viewport(viewport);
		auto window = viewport->window();

		m_window = Object::new_instance<ImGuiWindow>();
		m_window->initialize(window, EditorTheme::initialize_theme);
		m_viewport = viewport;

		return *this;
	}

	ImGuiEditorClient& ImGuiEditorClient::on_unbind_viewport(class RenderViewport* viewport)
	{
		Super::on_unbind_viewport(viewport);
		m_window->terminate();
		m_window   = nullptr;
		m_viewport = nullptr;

		return *this;
	}

	ImGuiEditorClient& ImGuiEditorClient::render(class RenderViewport* viewport)
	{
		Super::render(viewport);
		viewport->rhi_bind();
		viewport->rhi_clear_color(Color(0, 0, 0, 1));
		m_window->rhi_render();
		return *this;
	}

	ImGuiEditorClient& ImGuiEditorClient::update(class RenderViewport* viewport, float dt)
	{
		Super::update(viewport, dt);
		return *this;
	}

	ImGuiWindow* ImGuiEditorClient::imgui_window() const
	{
		return m_window.ptr();
	}

	const ImGuiEditorClient& ImGuiEditorClient::imgui_new_frame() const
	{
		m_window->new_frame();
		return *this;
	}

	const ImGuiEditorClient& ImGuiEditorClient::imgui_end_frame() const
	{
		m_window->end_frame();
		return *this;
	}

	Window* ImGuiEditorClient::window() const
	{
		return m_window ? m_window->window() : nullptr;
	}

	RenderViewport* ImGuiEditorClient::viewport() const
	{
		return m_viewport;
	}
}// namespace Engine
