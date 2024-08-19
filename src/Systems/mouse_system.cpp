#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <Systems/event_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	MouseSystem* MouseSystem::m_instance = nullptr;

	MouseSystem& MouseSystem::create()
	{
		Super::create();
		EventSystem* event_system = System::new_system<EventSystem>();
		event_system->register_subsystem(this);

		m_callbacks_identifier.push_back(event_system->add_listener(
				EventType::MouseMotion, std::bind(&MouseSystem::on_motion_event, this, std::placeholders::_1)));
		m_callbacks_identifier.push_back(event_system->add_listener(
				EventType::MouseButtonUp, std::bind(&MouseSystem::on_button_up_event, this, std::placeholders::_1)));
		m_callbacks_identifier.push_back(event_system->add_listener(
				EventType::MouseButtonDown, std::bind(&MouseSystem::on_button_down_event, this, std::placeholders::_1)));
		m_callbacks_identifier.push_back(event_system->add_listener(
				EventType::WindowClose, std::bind(&MouseSystem::on_window_close, this, std::placeholders::_1)));
		return *this;
	}

	static FORCE_INLINE Window* find_window(Identifier id)
	{
		auto manager = WindowManager::instance();
		if (manager)
		{
			return manager->find(id);
		}
		return nullptr;
	}

	void MouseSystem::on_motion_event(const Event& e)
	{
		const MouseMotionEvent& motion = e.get<const MouseMotionEvent&>();
		if (Window* window = find_window(e.window_id()))
		{
			auto& state				  = state_of(window);
			state.m_pos_info.x_offset = motion.xrel;
			state.m_pos_info.y_offset = motion.yrel;
			state.m_pos_info.x		  = motion.x;
			state.m_pos_info.y		  = motion.y;
		}
	}

	void MouseSystem::on_button_down_event(const Event& e)
	{
		const MouseButtonEvent& button_event = e.get<const MouseButtonEvent&>();
		if (Window* window = find_window(e.window_id()))
		{
			ButtonInfo& info = state_of(window).m_button_status[static_cast<EnumerateType>(button_event.button)];
			info.status		 = Mouse::JustPressed;
			info.x			 = button_event.x;
			info.y			 = button_event.y;
		}
	}

	void MouseSystem::on_button_up_event(const Event& e)
	{
		const MouseButtonEvent& button_event = e.get<const MouseButtonEvent&>();
		if (Window* window = find_window(e.window_id()))
		{
			ButtonInfo& info = state_of(window).m_button_status[static_cast<EnumerateType>(button_event.button)];
			info.status		 = Mouse::JustReleased;

			info.x = button_event.x;
			info.y = button_event.y;
		}
	}

	void MouseSystem::on_window_close(const Event& e)
	{
		if (Window* window = find_window(e.window_id()))
		{
			m_mouse_state.erase(window);
		}
	}

	void MouseSystem::on_wheel_event(const Event& e)
	{
		if (Window* window = find_window(e.window_id()))
		{
			const MouseWheelEvent& wheel = e.get<const MouseWheelEvent&>();
			auto& state					 = state_of(window);
			state.m_wheel_info.x		 = wheel.x;
			state.m_wheel_info.y		 = wheel.y;
		}
	}

	MouseSystem& MouseSystem::process_buttons(MouseState& state)
	{
		for (auto& info : state.m_button_status)
		{
			if (info.status == Mouse::JustPressed)
			{
				info.status = Mouse::Pressed;
			}
			else if (info.status == Mouse::JustReleased)
			{
				info.status = Mouse::Released;
			}

			info.x = -1;
			info.y = -1;
		}
		return *this;
	}

	MouseSystem::MouseState& MouseSystem::state_of(Window* window) const
	{
		if (window == nullptr)
		{
			auto instance = WindowManager::instance();
			window		  = instance->main_window();
		}

		return m_mouse_state[window];
	}

	MouseSystem& MouseSystem::wait()
	{
		Super::wait();
		return *this;
	}

	MouseSystem& MouseSystem::update(float dt)
	{
		Super::update(dt);
		for (auto& [window, state] : m_mouse_state)
		{
			state.m_pos_info.x_offset = 0;
			state.m_pos_info.y_offset = 0;

			state.m_wheel_info.x = state.m_wheel_info.y = 0.0f;
			process_buttons(state);
		}
		return *this;
	}

	MouseSystem& MouseSystem::shutdown()
	{
		Super::shutdown();

		EventSystem* system = EventSystem::instance();

		if (system)
		{
			for (auto& listener : m_callbacks_identifier)
			{
				system->remove_listener(listener);
			}
		}

		return *this;
	}

	const MouseSystem::PosInfo& MouseSystem::pos_info(Window* window) const
	{
		return state_of(window).m_pos_info;
	}

	bool MouseSystem::is_relative_mode(Window* window) const
	{
		return state_of(window).m_relative_mode;
	}

	MouseSystem& MouseSystem::relative_mode(bool flag, Window* window)
	{
		state_of(window).m_relative_mode = flag;
		WindowManager::instance()->mouse_relative_mode(flag);
		return *this;
	}

	bool MouseSystem::is_pressed(Mouse::Button button, Window* window) const
	{
		return state_of(window).m_button_status[button].status == Mouse::Pressed || is_just_pressed(button);
	}

	bool MouseSystem::is_released(Mouse::Button button, Window* window) const
	{
		return state_of(window).m_button_status[button].status == Mouse::Released || is_just_released(button);
	}

	bool MouseSystem::is_just_pressed(Mouse::Button button, Window* window) const
	{
		return state_of(window).m_button_status[button].status == Mouse::JustPressed;
	}

	bool MouseSystem::is_just_released(Mouse::Button button, Window* window) const
	{
		return state_of(window).m_button_status[button].status == Mouse::JustReleased;
	}

	const MouseSystem::ButtonInfo& MouseSystem::button_info(Mouse::Button button, Window* window) const
	{
		return state_of(window).m_button_status[button];
	}

	const MouseSystem::WheelInfo& MouseSystem::wheel_info(Window* window) const
	{
		return state_of(window).m_wheel_info;
	}

	implement_engine_class_default_init(MouseSystem, 0);
}// namespace Engine
