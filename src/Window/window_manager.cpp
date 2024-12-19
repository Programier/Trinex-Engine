#include "logo.hpp"
#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/constants.hpp>
#include <Core/exception.hpp>
#include <Core/library.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Image/image.hpp>
#include <Platform/platform.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>


namespace Engine
{
	static const Image& load_image_icon()
	{
		static Image image;

		if (image.empty())
		{
			image.load_from_memory(logo_png, logo_png_len, true);
		}

		return image;
	}

	WindowManager* WindowManager::m_instance = nullptr;

	WindowManager::WindowManager()
	{
		Platform::WindowManager::initialize();
	}

	WindowManager& WindowManager::destroy_window(Window* window)
	{
		if (window)
		{
			if (rhi)
			{
				render_thread()->wait();
			}

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

		const bool is_windows_empty = m_windows.empty();
		m_windows[window->id()]     = window;

		if (is_windows_empty)
		{
			m_main_window = window;

			static RHI* initialized_rhi = nullptr;

			if (initialized_rhi != rhi)
			{
				call_in_render_thread([window]() { rhi->initialize(window); });
			}
		}

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

	WindowManager& WindowManager::pool_events(void (*callback)(const Event& event, void* userdata), void* userdata)
	{
		Platform::WindowManager::pool_events(callback, userdata);
		return *this;
	}

	WindowManager& WindowManager::wait_for_events(void (*callback)(const Event& event, void* userdata), void* userdata)
	{
		Platform::WindowManager::wait_for_events(callback, userdata);
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
