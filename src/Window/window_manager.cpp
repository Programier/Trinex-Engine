#include "logo.hpp"
#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/constants.hpp>
#include <Core/library.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_viewport.hpp>
#include <Image/image.hpp>
#include <Platform/platform.hpp>
#include <RHI/rhi.hpp>
#include <Systems/event_system.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	static const Image& load_image_icon()
	{
		static Image image = {logo_png, logo_png_len};
		return image;
	}

	static void on_window_close(const Event& event)
	{
		WindowManager* manager = WindowManager::instance();

		if (manager == nullptr)
			return;

		Window* window = manager->find(event.window_id);

		if (manager->main_window() == window)
		{
			engine_instance->request_exit();
		}
		else if (window)
		{
			// Maybe the EventSystem will still send information about closing the window to other listeners.
			// Therefore, we will postpone the deletion of the window until the beginning of the next frame

			logic_thread()->add_task(Task(Task::High, [id = window->id()]() {
				// Let's check if the manager and the window still exist before deleting the window
				if (auto manager = WindowManager::instance())
				{
					if (auto window = manager->find(id))
					{
						manager->destroy_window(window);
					}
				}
			}));
		}
	}

	static void on_resize(const Event& event)
	{
		WindowManager* manager = WindowManager::instance();
		Window* window         = manager->find(event.window_id);
		if (!window)
			return;

		{
			auto x                                = event.window.x;
			auto y                                = event.window.y;
			WindowRenderViewport* render_viewport = window->render_viewport();
			if (render_viewport)
			{
				render_viewport->on_resize({x, y});
			}
		}
	}

	static void on_orientation_changed(const Event& event)
	{
		WindowManager* manager = WindowManager::instance();
		Window* window         = manager->find(event.window_id);
		if (!window)
			return;

		{
			WindowRenderViewport* render_viewport = window->render_viewport();
			if (render_viewport)
			{
				render_viewport->on_orientation_changed(event.display.orientation);
			}
		}
	}


	WindowManager* WindowManager::s_instance = nullptr;

	WindowManager::WindowManager()
	{
		Platform::WindowManager::initialize();
		EventSystem* event_system = System::system_of<EventSystem>();

		event_system->add_listener(EventType::WindowClose, &on_window_close);
		event_system->add_listener(EventType::WindowResized, on_resize);
		event_system->add_listener(EventType::DisplayOrientationChanged, on_orientation_changed);
	}

	WindowManager& WindowManager::destroy_window(Window* window)
	{
		if (window)
		{
			if (window == m_main_window)
				m_main_window = nullptr;
			m_windows.erase(window->id());

			while (!window->m_childs.empty())
			{
				destroy_window(window->m_childs.back());
			}

			if (window->m_parent_window)
			{
				auto& childs = window->m_parent_window->m_childs;
				for (size_t i = 0, count = childs.size(); i < count; i++)
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
		return *this;
	}

	WindowManager::~WindowManager()
	{
		while (!m_windows.empty())
		{
			Window* window = m_windows.begin()->second;
			destroy_window(window);
		}

		Platform::WindowManager::terminate();
	}

	Window* WindowManager::create_window(const WindowConfig& config, Window* parent, Window* window)
	{
		if (window == nullptr)
			window = Platform::WindowManager::create_window(&config);

		if (window == nullptr)
			return nullptr;

		parent = parent ? parent : m_main_window;
		if (parent)
		{
			parent->m_childs.push_back(window);
			window->m_parent_window = parent;
		}

		if (m_windows.empty())
			m_main_window = window;

		m_windows[window->id()] = window;

		// Initialize client
		window->icon(load_image_icon());
		window->initialize(config);
		return window;
	}

	WindowManager& WindowManager::mouse_relative_mode(bool flag)
	{
		Platform::WindowManager::mouse_relative_mode(flag);
		return *this;
	}

	Window* WindowManager::find(Identifier id) const
	{
		auto it = m_windows.find(id);
		if (it == m_windows.end())
			return nullptr;
		return it->second;
	}

	Window* WindowManager::main_window() const
	{
		return m_main_window;
	}

	const TreeMap<Identifier, Window*>& WindowManager::windows() const
	{
		return m_windows;
	}
}// namespace Engine
