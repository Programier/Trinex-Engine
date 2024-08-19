#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <Systems/event_system.hpp>
#include <Systems/touchscreen_system.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	implement_engine_class_default_init(TouchScreenSystem, 0);

	TouchScreenSystem& TouchScreenSystem::create()
	{
		Super::create();

		EventSystem* event_system = System::new_system<EventSystem>();
		event_system->register_subsystem(this);

		m_listeners = {
				event_system->add_listener(EventType::FingerDown, std::bind(&This::on_finger_down, this, std::placeholders::_1)),
				event_system->add_listener(EventType::FingerUp, std::bind(&This::on_finger_up, this, std::placeholders::_1)),
				event_system->add_listener(EventType::FingerMotion,
										   std::bind(&This::on_finger_motion, this, std::placeholders::_1)),
		};
		return *this;
	}

	TouchScreenSystem& TouchScreenSystem::update(float dt)
	{
		Super::update(dt);
		return *this;
	}

	TouchScreenSystem& TouchScreenSystem::shutdown()
	{
		if (EventSystem* system = EventSystem::instance())
		{
			for (auto& listener : m_listeners)
			{
				system->remove_listener(listener);
			}
		}

		Super::shutdown();
		return *this;
	}

	Vector<TouchScreenSystem::Finger>& TouchScreenSystem::find_fingers_data(Window* window) const
	{
		if (window == nullptr)
			window = WindowManager::instance()->main_window();

		return m_fingers[window];
	}

	size_t TouchScreenSystem::finger_count(Window* window) const
	{
		return find_fingers_data(window).size();
	}

	bool TouchScreenSystem::is_finger_down(size_t finger_index, Window* window) const
	{
		auto& data = find_fingers_data(window);
		if (data.size() <= finger_index)
			return false;
		return data[finger_index].is_down;
	}

	Vector2D TouchScreenSystem::finger_location(size_t finger_index, Window* window) const
	{
		auto& data = find_fingers_data(window);
		if (data.size() <= finger_index)
			return {-1, -1};
		return {data[finger_index].x, data[finger_index].y};
	}

	Vector2D TouchScreenSystem::finger_offset(size_t finger_index, Window* window) const
	{
		auto& data = find_fingers_data(window);
		if (data.size() <= finger_index)
			return {0.f, 0.f};
		return {data[finger_index].x_offset, data[finger_index].y_offset};
	}

	const TouchScreenSystem::Finger& TouchScreenSystem::finger_info(size_t finger_index, Window* window) const
	{
		auto& data = find_fingers_data(window);
		if (data.size() <= finger_index)
		{
			static const Finger finger = {};
			return finger;
		}
		return data[finger_index];
	}

	static FORCE_INLINE Vector<TouchScreenSystem::Finger>& validate_fingers(Vector<TouchScreenSystem::Finger>& data,
																			Index finger_index)
	{
		if (data.size() <= finger_index)
			data.resize(finger_index + 1);
		return data;
	}

	void TouchScreenSystem::on_finger_up(const Event& event)
	{
		if (auto window = WindowManager::instance()->find(event.window_id()))
		{
			const FingerUpEvent& event_data = event.get<const FingerUpEvent&>();
			auto& data	 = validate_fingers(find_fingers_data(window), event_data.finger_index)[event_data.finger_index];
			data.is_down = false;
			data.x = data.y = -1.f;
			data.x_offset = data.y_offset = 0.f;
		}
	}

	void TouchScreenSystem::on_finger_down(const Event& event)
	{
		if (auto window = WindowManager::instance()->find(event.window_id()))
		{
			const FingerDownEvent& event_data = event.get<const FingerDownEvent&>();
			auto& data	  = validate_fingers(find_fingers_data(window), event_data.finger_index)[event_data.finger_index];
			data.is_down  = true;
			data.x		  = event_data.x;
			data.y		  = event_data.y;
			data.x_offset = data.y_offset = 0.f;
		}
	}

	void TouchScreenSystem::on_finger_motion(const Event& event)
	{
		if (auto window = WindowManager::instance()->find(event.window_id()))
		{
			const FingerMotionEvent& event_data = event.get<const FingerMotionEvent&>();
			auto& data	  = validate_fingers(find_fingers_data(window), event_data.finger_index)[event_data.finger_index];
			data.is_down  = true;
			data.x		  = event_data.x;
			data.y		  = event_data.y;
			data.x_offset = event_data.xrel;
			data.y_offset = event_data.yrel;
		}
	}
}// namespace Engine
