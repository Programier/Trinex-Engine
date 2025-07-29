#include <Core/base_engine.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{

	float_t Window::width()
	{
		return m_size.load().x;
	}

	Window& Window::width(float_t width)
	{
		return *this;
	}

	float_t Window::height()
	{
		return m_size.load().y;
	}

	Window& Window::height(float_t height)
	{
		return *this;
	}

	Vector2u Window::size()
	{
		return m_size.load();
	}

	Window& Window::size(const Vector2u& size)
	{
		return *this;
	}

	String Window::title()
	{
		return "Trinex Engine Window";
	}

	Window& Window::title(const String& title)
	{
		return *this;
	}

	Vector2u Window::position()
	{
		return {0, 0};
	}

	Window& Window::position(const Vector2u& position)
	{
		return *this;
	}

	bool Window::resizable()
	{
		return false;
	}

	Window& Window::resizable(bool value)
	{
		return *this;
	}

	Window& Window::focus()
	{
		return *this;
	}

	bool Window::focused()
	{
		return false;
	}

	Window& Window::show()
	{
		return *this;
	}

	Window& Window::hide()
	{
		return *this;
	}

	bool Window::is_visible()
	{
		return false;
	}

	bool Window::is_iconify()
	{
		return false;
	}

	Window& Window::iconify()
	{
		return *this;
	}

	bool Window::is_restored()
	{
		return false;
	}

	Window& Window::restore()
	{
		return *this;
	}

	Window& Window::opacity(float value)
	{
		return *this;
	}

	float Window::opacity()
	{
		return 1.f;
	}

	Window& Window::icon(const Image& image)
	{
		return *this;
	}

	Window& Window::cursor(const Image& image, Vector2i hotspot)
	{
		return *this;
	}

	Window& Window::attribute(const WindowAttribute& attrib, bool value)
	{
		return *this;
	}

	bool Window::attribute(const WindowAttribute& attrib)
	{
		return false;
	}

	Window& Window::cursor_mode(const CursorMode& mode)
	{
		return *this;
	}

	CursorMode Window::cursor_mode()
	{
		return CursorMode::Normal;
	}

	bool Window::support_orientation(Orientation orientation)
	{
		return false;
	}

	Orientation Window::orientation()
	{
		return Orientation::Landscape;
	}

	Identifier Window::id()
	{
		return reinterpret_cast<Identifier>(this);
	}

	void* Window::native_window()
	{
		return nullptr;
	}

	size_t Window::monitor_index()
	{
		return 0;
	}

	void Window::initialize(const WindowConfig& config)
	{
		m_render_viewport = Object::new_instance<WindowRenderViewport>("", nullptr, this, config.vsync);

		if (!InitializeController().is_triggered())
		{
			// Default resources is not loaded now, so, using deferred initialization
			InitializeController().push([this, client = config.client]() { create_client(client); });
		}
		else
		{
			create_client(config.client);
		}
	}

	WindowRenderViewport* Window::render_viewport() const
	{
		return m_render_viewport;
	}

	Window* Window::parent_window() const
	{
		return m_parent_window;
	}

	const Vector<Window*>& Window::child_windows() const
	{
		return m_childs;
	}

	Window::~Window()
	{
		if (m_render_viewport)
		{
			RenderViewport* viewport = m_render_viewport;
			m_render_viewport        = nullptr;
			viewport->client(nullptr);
			GarbageCollector::destroy(viewport);
		}
	}

	Window& Window::create_client(const StringView& client_name)
	{
		ViewportClient* client = ViewportClient::create(client_name);
		if (client)
		{
			render_viewport()->client(client);
		}
		return *this;
	}
}// namespace Engine
