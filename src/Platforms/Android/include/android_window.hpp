#pragma once
#include <Core/etl/set.hpp>
#include <EGL/egl.h>
#include <Window/window.hpp>

struct ANativeWindow;
struct AInputEvent;
struct ImGuiContext;

namespace Engine
{
	class ENGINE_EXPORT AndroidWindow : public Window
	{
	protected:
		String m_name         = "Android Window";
		bool m_init_vsync : 1 = true;

	public:
		using Window::m_size;
		struct ImGuiContext* imgui_context = nullptr;

		static ANativeWindow* static_native_window();

		using Window::height;
		using Window::size;
		using Window::width;

		Vector2u calc_size() const;
		void resized();

		void initialize(const WindowConfig&) override;
		Window& width(float_t width) override;
		Window& height(float_t height) override;
		Window& size(const Vector2u& size) override;
		String title() override;
		Window& title(const String& title) override;
		Vector2u position() override;
		Window& position(const Vector2u& position) override;
		bool resizable() override;
		Window& resizable(bool value) override;
		Window& focus() override;
		bool focused() override;
		Window& show() override;
		Window& hide() override;
		bool is_visible() override;
		bool is_iconify() override;
		Window& iconify() override;
		bool is_restored() override;
		Window& restore() override;
		Window& opacity(float value) override;
		float opacity() override;
		Window& icon(const Image& image) override;
		Window& cursor(const Image& image, Vector2i hotspot = {0, 0}) override;
		Window& attribute(const WindowAttribute& attrib, bool value) override;
		bool attribute(const WindowAttribute& attrib) override;
		Window& cursor_mode(const CursorMode& mode) override;
		CursorMode cursor_mode() override;
		bool support_orientation(Orientation orientation) override;
		Orientation orientation() override;
		Identifier id() override;
		void* native_window() override;
		size_t monitor_index() override;

		~AndroidWindow();
	};
}// namespace Engine
