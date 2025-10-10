#include <Core/exception.hpp>
#include <EGL/egl.h>
#include <Window/config.hpp>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <android_platform.hpp>
#include <android_window.hpp>

#include <vulkan/vulkan.h>

namespace Engine
{
	AndroidWindow::~AndroidWindow()
	{
		ANativeWindow_release(static_native_window());
	}


	ANativeWindow* AndroidWindow::static_native_window()
	{
		return Platform::android_application()->window;
	}

	Vector2u AndroidWindow::calc_size() const
	{
		auto window = static_native_window();
		return {static_cast<float>(ANativeWindow_getWidth(window)), static_cast<float>(ANativeWindow_getHeight(window))};
	}

	String AndroidWindow::title()
	{
		return m_name;
	}

	Window& AndroidWindow::title(const String& title)
	{
		m_name = title;
		return *this;
	}

	Vector2u AndroidWindow::position()
	{
		return {0.f, 0.f};
	}

	Window& AndroidWindow::position(const Vector2u& position)
	{
		return *this;
	}

	Window& AndroidWindow::width(float_t value)
	{
		size(Vector2u(value, height()));
		return *this;
	}

	Window& AndroidWindow::height(float_t value)
	{
		size(Vector2u(height(), value));
		return *this;
	}

	Window& AndroidWindow::size(const Vector2u& size)
	{
		if (resizable())
		{
			uint32_t x = static_cast<uint32_t>(size.x);
			uint32_t y = static_cast<uint32_t>(size.y);

			ANativeWindow* window = static_native_window();
			ANativeWindow_setBuffersGeometry(window, x, y, ANativeWindow_getFormat(window));
		}
		return *this;
	}

	bool AndroidWindow::resizable()
	{
		return false;
	}

	Window& AndroidWindow::resizable(bool value)
	{
		return *this;
	}

	Identifier AndroidWindow::id()
	{
		return reinterpret_cast<Identifier>(native_window());
	}


	Window& AndroidWindow::focus()
	{
		return *this;
	}

	bool AndroidWindow::focused()
	{
		return true;
	}

	Window& AndroidWindow::show()
	{
		return *this;
	}

	Window& AndroidWindow::hide()
	{
		return *this;
	}

	bool AndroidWindow::is_visible()
	{
		return true;
	}

	bool AndroidWindow::is_iconify()
	{
		return false;
	}

	Window& AndroidWindow::iconify()
	{
		return *this;
	}

	bool AndroidWindow::is_restored()
	{
		return true;
	}

	Window& AndroidWindow::restore()
	{
		return *this;
	}

	Window& AndroidWindow::opacity(float value)
	{
		return *this;
	}

	float AndroidWindow::opacity()
	{
		return 1.f;
	}

	void* AndroidWindow::native_window()
	{
		return static_native_window();
	}

	void AndroidWindow::resized()
	{
		ANativeWindow_setBuffersGeometry(static_native_window(), width(), height(),
		                                 ANativeWindow_getFormat(static_native_window()));
	}

	Window& AndroidWindow::icon(const Image& image)
	{
		return *this;
	}

	Window& AndroidWindow::cursor(const Image& image, Vector2i hotspot)
	{
		return *this;
	}

	Window& AndroidWindow::attribute(const WindowAttribute& attrib, bool value)
	{
		return *this;
	}

	bool AndroidWindow::attribute(const WindowAttribute& attrib)
	{
		return false;
	}

	Window& AndroidWindow::cursor_mode(const CursorMode& mode)
	{
		return *this;
	}

	CursorMode AndroidWindow::cursor_mode()
	{
		return CursorMode::Normal;
	}

	bool AndroidWindow::support_orientation(Orientation orientation)
	{
		return true;
	}

	Orientation AndroidWindow::orientation()
	{
		return Platform::m_android_platform_info.orientation;
	}

	void AndroidWindow::initialize(const WindowConfig& config)
	{
		ANativeWindow_acquire(static_native_window());
		m_init_vsync = config.vsync;

		m_size.store(calc_size());

		Window::initialize(config);
	}

	size_t AndroidWindow::monitor_index()
	{
		return 0;
	}
}// namespace Engine
