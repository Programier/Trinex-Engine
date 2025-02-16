#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/mouse.hpp>
#include <Systems/system.hpp>

namespace Engine
{
	class Window;
	struct Event;

	class ENGINE_EXPORT MouseSystem : public Singletone<MouseSystem, System>
	{
		declare_class(MouseSystem, System);

	public:
		struct PosInfo {
			float x;
			float y;
			float x_offset;
			float y_offset;
		};

		struct ButtonInfo {
			Mouse::Status status = Mouse::Status::Released;
			float x              = 0;
			float y              = 0;
		};

		struct WheelInfo {
			float x;
			float y;
		};


	private:
		struct MouseState {
			PosInfo m_pos_info;
			WheelInfo m_wheel_info;
			ButtonInfo m_button_status[static_cast<EnumerateType>(Mouse::Button::__COUNT__)];
			bool m_relative_mode = false;
		};

		mutable Map<Window*, MouseState> m_mouse_state;
		static MouseSystem* m_instance;
		Vector<Identifier> m_callbacks_identifier;

		void on_motion_event(const Event& e);
		void on_button_down_event(const Event& e);
		void on_button_up_event(const Event& e);
		void on_wheel_event(const Event& e);
		void on_window_close(const Event& e);

		MouseSystem& process_buttons(MouseState& state);
		MouseState& state_of(Window* window) const;

	protected:
		MouseSystem& create() override;

	public:
		MouseSystem& wait() override;
		MouseSystem& update(float dt) override;
		MouseSystem& shutdown() override;
		const PosInfo& pos_info(Window* window = nullptr) const;

		bool is_relative_mode(Window* window = nullptr) const;
		MouseSystem& relative_mode(bool flag, Window* window = nullptr);
		bool is_pressed(Mouse::Button button, Window* window = nullptr) const;
		bool is_released(Mouse::Button button, Window* window = nullptr) const;
		bool is_just_pressed(Mouse::Button button, Window* window = nullptr) const;
		bool is_just_released(Mouse::Button button, Window* window = nullptr) const;
		const ButtonInfo& button_info(Mouse::Button button, Window* window = nullptr) const;
		const WheelInfo& wheel_info(Window* window = nullptr) const;

		friend class Singletone<MouseSystem, System>;
	};

}// namespace Engine
