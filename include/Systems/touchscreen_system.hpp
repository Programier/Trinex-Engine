#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Systems/system.hpp>


namespace Engine
{
	class Window;
	struct Event;

	class ENGINE_EXPORT TouchScreenSystem : public Singletone<TouchScreenSystem, System>
	{
		trinex_declare_class(TouchScreenSystem, System);

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
		Identifier m_listeners[3];

		void on_finger_up(const Event& event);
		void on_finger_down(const Event& event);
		void on_finger_motion(const Event& event);

	protected:
		TouchScreenSystem& create() override;

	public:
		TouchScreenSystem& update(float dt) override;
		TouchScreenSystem& shutdown() override;

		size_t finger_count(Window* window = nullptr) const;
		bool is_finger_down(size_t finger_index, Window* window = nullptr) const;
		Vector2f finger_location(size_t finger_index, Window* window = nullptr) const;
		Vector2f finger_offset(size_t finger_index, Window* window = nullptr) const;
		const Finger& finger_info(size_t finger_index, Window* window = nullptr) const;
	};
}// namespace Engine
