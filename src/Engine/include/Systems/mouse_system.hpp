#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/mouse.hpp>
#include <Core/system.hpp>
#include <Event/event.hpp>

namespace Engine
{
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
            Mouse::Status status;
            int_t x;
            int_t y;
            int_t clicks;
        };

        struct WheelInfo {
            Mouse::Direction direction;
            float x;
            float y;
        };


    private:
        PosInfo _M_pos_info;
        WheelInfo _M_wheel_info;

        static MouseSystem* _M_instance;
        ButtonInfo _M_button_status[static_cast<EnumerateType>(Mouse::Button::__COUNT__)];
        Identifier _M_callbacks_identifier[4];

        void on_motion_event(const Event& e);
        void on_button_down_event(const Event& e);
        void on_button_up_event(const Event& e);
        void on_wheel_event(const Event& e);

        MouseSystem& process_buttons();

    public:
        MouseSystem& create() override;
        MouseSystem& wait() override;
        MouseSystem& update(float dt) override;
        MouseSystem& shutdown() override;
        const PosInfo& pos_info() const;

        bool relative_mode() const;
        MouseSystem& relative_mode(bool flag);
        bool is_pressed(Mouse::Button button) const;
        bool is_released(Mouse::Button button) const;
        bool is_just_pressed(Mouse::Button button) const;
        bool is_just_released(Mouse::Button button) const;
        const ButtonInfo& button_info(Mouse::Button button) const;
        const WheelInfo& wheel_info() const;

        friend class Object;
    };

}// namespace Engine
