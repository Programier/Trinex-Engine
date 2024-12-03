#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/singletone.hpp>
#include <Event/listener_id.hpp>
#include <Systems/system.hpp>


namespace Engine
{
	class Window;

	class ENGINE_EXPORT TouchScreenSystem : public Singletone<TouchScreenSystem, System>
	{
		declare_class(TouchScreenSystem, System);

	public:
		struct ENGINE_EXPORT Finger {
			bool is_down   = false;
			float x        = -1.f;
			float y        = -1.f;
			float x_offset = 0.f;
			float y_offset = 0.f;
		};

	private:
		mutable TreeMap<Window*, Vector<Finger>> m_fingers;
		Vector<Finger>& find_fingers_data(Window* window) const;
		Array<EventSystemListenerID, 3> m_listeners;

		void on_finger_up(const Event& event);
		void on_finger_down(const Event& event);
		void on_finger_motion(const Event& event);

	public:
		TouchScreenSystem& create() override;
		TouchScreenSystem& update(float dt) override;
		TouchScreenSystem& shutdown() override;

		size_t finger_count(Window* window = nullptr) const;
		bool is_finger_down(size_t finger_index, Window* window = nullptr) const;
		Vector2D finger_location(size_t finger_index, Window* window = nullptr) const;
		Vector2D finger_offset(size_t finger_index, Window* window = nullptr) const;
		const Finger& finger_info(size_t finger_index, Window* window = nullptr) const;
	};
}// namespace Engine
