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
		String m_name;
		bool m_init_vsync : 1 = true;

	public:
		using Window::m_size;
		struct ImGuiContext* imgui_context = nullptr;

		AndroidWindow(const WindowConfig* config);

		static ANativeWindow* static_native_window();

		using Window::height;
		using Window::size;
		using Window::width;

		Size2D calc_size() const;

		String title() override;
		Window& title(const String& title) override;
		Point2D position() override;
		Window& position(const Point2D& position) override;
		Window& width(float_t width) override;
		Window& height(float_t height) override;
		Window& size(const Size2D& size) override;
		bool resizable() override;
		Window& resizable(bool value) override;
		Identifier id() override;

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
		void* native_window() override;

		virtual void resized();

		Window& icon(const Image& image) override;
		Window& cursor(const Image& image, Vector2i hotspot = {0, 0}) override;
		Window& attribute(const WindowAttribute& attrib, bool value) override;
		bool attribute(const WindowAttribute& attrib) override;
		Window& cursor_mode(const CursorMode& mode) override;
		CursorMode cursor_mode() override;
		bool support_orientation(Orientation orientation) override;
		Orientation orientation() override;
		~AndroidWindow();
	};


	struct AndroidEGLSurface {
		EGLSurface egl_surface            = EGL_NO_SURFACE;
		struct AndroidEGLContext* context = nullptr;

		void init(AndroidEGLContext* context, ANativeWindow* window);
		void destroy();
		~AndroidEGLSurface();
	};

	struct ENGINE_EXPORT AndroidEGLContext {
		EGLDisplay egl_display = EGL_NO_DISPLAY;
		EGLContext egl_context = EGL_NO_CONTEXT;
		EGLConfig egl_config   = nullptr;
		EGLint egl_format      = 0;
		Set<AndroidEGLSurface*> egl_surfaces;
		bool m_vsync = false;


		AndroidEGLContext();
		bool vsync();
		AndroidEGLContext& vsync(bool flag);
		~AndroidEGLContext();
	};

	class ENGINE_EXPORT AndroidEGLWindow : public AndroidWindow
	{
		AndroidEGLSurface m_surface;

	public:
		AndroidEGLWindow(const WindowConfig* config);
		~AndroidEGLWindow();

		void* create_context();
		EGLSurface surface(AndroidEGLContext* context);
		AndroidEGLWindow& make_current(void* context);
		AndroidEGLWindow& swap_buffers(void* context);
		AndroidEGLWindow& destroy_context(void* context);
	};

	class ENGINE_EXPORT AndroidVulkanWindow : public AndroidWindow
	{
	public:
		AndroidVulkanWindow(const WindowConfig* config);
	};
}// namespace Engine
