#include <Core/base_engine.hpp>
#include <Core/console.hpp>
#include <Core/etl/map.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/viewport_client.hpp>
#include <Core/window.hpp>
#include <Graphics/render_viewport.hpp>
#include <Input/event_system.hpp>
#include <Platform/platform.hpp>

namespace Trinex
{
	struct WindowEventListener final : EventListener {
		EventDispatchResult on_event(RoutedEvent& event) override
		{
			auto* payload = reinterpret_cast<const WindowEvent*>(event.payload);
			if (payload == nullptr)
				return {};

			Window* window = Window::find(event.header.window_id);

			if (window == nullptr)
				return {};

			switch (payload->kind)
			{
				case WindowEventKind::Resized:
				{
					if (RenderViewport* viewport = window->render_viewport())
					{
						viewport->on_resize({payload->size.x, payload->size.y});
					}
					break;
				}

				case WindowEventKind::CloseRequested:
				{
					if (Window::main() == window)
					{
						engine_instance->request_exit();
					}

					logic_thread()->add_task(Task(Task::High, [id = window->id()]() {
						if (auto* window = Window::find(id))
						{
							Window::destroy(window);
						}
					}));
					break;
				}

				default: break;
			}

			return {};
		}
	};

	static WindowEventListener s_window_event_listener;
	static WindowDesc s_config;

	trinex_on_pre_init()
	{
		static Console::VariableRef title(&s_config.title, "window.title");
		static Console::VariableRef client(&s_config.client, "window.client");
		static Console::VariableRef size(&s_config.size, "window.size");
		static Console::VariableRef pos(&s_config.pos, "window.pos");
		static Console::VariableRef monitor(&s_config.monitor, "window.monitor");

		// WindowAttribute attributes = WindowAttribute::Undefined;
	}

	struct WindowsState {
		Window* main = nullptr;
		Map<Identifier, Window*> windows;

		static WindowsState& instance()
		{
			static WindowsState s_state = []() {
				Platform::WindowManager::initialize();

				if (EventSystem* event_system = EventSystem::instance())
				{
					event_system->dispatcher().add_listener(EventTypeIds::Window, &s_window_event_listener);
				}

				LifeCycle::on_post_shutdown([]() {
					auto& windows = WindowsState::instance().windows;

					while (!windows.empty())
					{
						Window* window = windows.begin()->second;
						Window::destroy(window);
					}

					Platform::WindowManager::terminate();
				});


				WindowsState state;
				return state;
			}();

			return s_state;
		}
	};

	ENGINE_EXPORT const WindowDesc& WindowDesc::from_config()
	{
		return s_config;
	}

	Window* Window::create(String title, Vector2u size, Window* parent, Window* self)
	{
		WindowDesc desc = {
		        .title = title,
		        .size  = size,
		};

		return create(desc, parent, self);
	}

	Window* Window::create(const WindowDesc& desc, Window* parent, Window* self)
	{
		if (self == nullptr)
			self = Platform::WindowManager::create_window(&desc);

		if (self == nullptr)
			return nullptr;

		parent = parent ? parent : WindowsState::instance().main;

		if (parent)
		{
			parent->m_childs.push_back(self);
			self->m_parent_window = parent;
		}

		if (WindowsState::instance().main == nullptr)
			WindowsState::instance().main = self;

		WindowsState::instance().windows[self->id()] = self;

		// Initialize client
		//self->icon(load_image_icon());
		return self;
	}

	void Window::destroy(Window* window)
	{
		if (window)
		{
			if (window == WindowsState::instance().main)
				WindowsState::instance().main = nullptr;

			WindowsState::instance().windows.erase(window->id());

			while (!window->m_childs.empty())
			{
				destroy(window->m_childs.back());
			}

			if (window->m_parent_window)
			{
				auto& childs = window->m_parent_window->m_childs;
				for (usize i = 0, count = childs.size(); i < count; i++)
				{
					if (childs[i] == window)
					{
						childs.erase(childs.begin() + i);
						break;
					}
				}
			}

			window->on_destroy(window);
			Platform::WindowManager::destroy_window(window);
		}
	}

	Window* Window::find(Identifier id)
	{
		auto it = WindowsState::instance().windows.find(id);

		if (it == WindowsState::instance().windows.end())
			return nullptr;

		return it->second;
	}

	Window* Window::main()
	{
		return WindowsState::instance().main;
	}

	f32 Window::width()
	{
		return m_size.load().x;
	}

	Window& Window::width(f32 width)
	{
		return *this;
	}

	f32 Window::height()
	{
		return m_size.load().y;
	}

	Window& Window::height(f32 height)
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

	usize Window::monitor_index()
	{
		return 0;
	}

	RenderViewport* Window::render_viewport() const
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
}// namespace Trinex
