#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/mouse.hpp>
#include <Event/listener_id.hpp>
#include <Systems/system.hpp>

namespace Engine
{
    class Window;
    class ENGINE_EXPORT MouseSystem : public Singletone<MouseSystem, System>
    {
        declare_class(MouseSystem, System);

    public:
        struct PosInfo {
            int_t x;
            int_t y;
            int_t x_offset;
            int_t y_offset;
        };

        struct ButtonInfo {
            Mouse::Status status = Mouse::Status::Released;
            int_t x              = 0;
            int_t y              = 0;
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
        };

        mutable Map<Window*, MouseState> m_mouse_state;
        static MouseSystem* m_instance;
        Vector<EventSystemListenerID> m_callbacks_identifier;

        void on_motion_event(const Event& e);
        void on_button_down_event(const Event& e);
        void on_button_up_event(const Event& e);
        void on_wheel_event(const Event& e);
        void on_window_close(const Event& e);

        MouseSystem& process_buttons(MouseState& state);
        MouseState& state_of(Window* window) const;

    public:
        MouseSystem& create() override;
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
