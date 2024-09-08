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
	AndroidWindow::AndroidWindow(const WindowConfig* config) : m_name("Android Window")
	{
		ANativeWindow_acquire(static_native_window());
		m_init_vsync = config->vsync;

		m_size.store(calc_size());
	}

	AndroidWindow::~AndroidWindow()
	{
		ANativeWindow_release(static_native_window());
	}

	Size2D AndroidWindow::calc_size() const
	{
		auto window = static_native_window();
		return {static_cast<float>(ANativeWindow_getWidth(window)), static_cast<float>(ANativeWindow_getHeight(window))};
	}

	ANativeWindow* AndroidWindow::static_native_window()
	{
		return Platform::android_application()->window;
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

	Point2D AndroidWindow::position()
	{
		return {0.f, 0.f};
	}

	Window& AndroidWindow::position(const Point2D& position)
	{
		return *this;
	}

	Window& AndroidWindow::width(const Size1D& value)
	{
		size(Size2D(value, height()));
		return *this;
	}

	Window& AndroidWindow::height(const Size1D& value)
	{
		size(Size2D(height(), value));
		return *this;
	}

	Window& AndroidWindow::size(const Size2D& size)
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

	Window& AndroidWindow::cursor(const Image& image, IntVector2D hotspot)
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

	//// EGL WINDOW

	void AndroidEGLSurface::init(AndroidEGLContext* _context, ANativeWindow* window)
	{
		context     = _context;
		egl_surface = eglCreateWindowSurface(context->egl_display, context->egl_config, window, nullptr);
		context->egl_surfaces.insert(this);
	}

	void AndroidEGLSurface::destroy()
	{
		if (context && egl_surface != EGL_NO_SURFACE)
		{
			eglDestroySurface(context->egl_display, egl_surface);
			context->egl_surfaces.erase(this);
			context     = nullptr;
			egl_surface = EGL_NO_SURFACE;
		}
	}

	AndroidEGLSurface::~AndroidEGLSurface()
	{
		destroy();
	}

	AndroidEGLContext::AndroidEGLContext()
	{
		egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

		if (egl_display == EGL_NO_DISPLAY)
		{
			throw EngineException("eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");
		}

		if (eglInitialize(egl_display, 0, 0) != EGL_TRUE)
		{
			throw EngineException("eglInitialize() returned with an error");
		}


		const EGLint egl_attributes[] = {
		        EGL_BLUE_SIZE,  8,       EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE,
		        EGL_WINDOW_BIT, EGL_NONE};
		EGLint num_configs = 0;
		if (eglChooseConfig(egl_display, egl_attributes, nullptr, 0, &num_configs) != EGL_TRUE)
			throw EngineException("eglChooseConfig() returned with an error");

		if (num_configs == 0)
			throw EngineException("eglChooseConfig() returned 0 matching config");

		eglChooseConfig(egl_display, egl_attributes, &egl_config, 1, &num_configs);
		eglGetConfigAttrib(egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);

		const EGLint egl_context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
		egl_context                           = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, egl_context_attributes);

		if (egl_context == EGL_NO_CONTEXT)
			throw EngineException("eglCreateContext() returned EGL_NO_CONTEXT");
	}

	bool AndroidEGLContext::vsync()
	{
		return m_vsync;
	}

	AndroidEGLContext& AndroidEGLContext::vsync(bool flag)
	{
		eglSwapInterval(egl_display, flag ? 1 : 0);
		return *this;
	}

	AndroidEGLContext::~AndroidEGLContext()
	{
		if (egl_display != EGL_NO_DISPLAY)
		{
			if (egl_context != EGL_NO_CONTEXT)
				eglDestroyContext(egl_display, egl_context);

			while (!egl_surfaces.empty())
			{
				AndroidEGLSurface* surface_ptr = *egl_surfaces.begin();
				surface_ptr->destroy();
			}

			eglTerminate(egl_display);

			egl_display = EGL_NO_DISPLAY;
			egl_context = EGL_NO_CONTEXT;
		}
	}

	AndroidEGLWindow::AndroidEGLWindow(const WindowConfig* config) : AndroidWindow(config)
	{}

	AndroidEGLWindow::~AndroidEGLWindow()
	{}

	EGLSurface AndroidEGLWindow::surface(AndroidEGLContext* context)
	{
		if (m_surface.context == EGL_NO_SURFACE)
		{
			ANativeWindow_setBuffersGeometry(static_native_window(), width(), height(), context->egl_format);
			m_surface.init(context, reinterpret_cast<ANativeWindow*>(native_window()));
		}

		return m_surface.egl_surface;
	}

	AndroidEGLWindow& AndroidEGLWindow::make_current(void* _context)
	{
		AndroidEGLContext* context = reinterpret_cast<AndroidEGLContext*>(_context);
		EGLSurface egl_surface     = surface(context);
		eglMakeCurrent(context->egl_display, egl_surface, egl_surface, context->egl_context);
		return *this;
	}

	AndroidEGLWindow& AndroidEGLWindow::swap_buffers(void* _context)
	{
		AndroidEGLContext* context = reinterpret_cast<AndroidEGLContext*>(_context);
		eglSwapBuffers(context->egl_display, surface(context));
		return *this;
	}

	void* AndroidEGLWindow::create_context()
	{
		return new AndroidEGLContext();
	}

	AndroidEGLWindow& AndroidEGLWindow::destroy_context(void* context)
	{
		delete reinterpret_cast<AndroidEGLContext*>(context);
		return *this;
	}

	// Vulkan Window

	AndroidVulkanWindow::AndroidVulkanWindow(const WindowConfig* config) : AndroidWindow(config)
	{}

	// void* AndroidVulkanWindow::create_api_context(const char* any_text, ...)
	// {
	//     va_list args;
	//     va_start(args, any_text);
	//     VkInstance instance = va_arg(args, VkInstance);
	//     va_end(args);

	//     ANativeWindow* window = static_native_window();
	//     VkSurfaceKHR surface;
	//     VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
	//     surfaceCreateInfo.sType                         = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	//     surfaceCreateInfo.window                        = window;

	//     if (vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS)
	//     {
	//         throw EngineException("Android: Failed to create Vulkan Surface");
	//     }

	//     return reinterpret_cast<void*>(surface);
	// }
}// namespace Engine
