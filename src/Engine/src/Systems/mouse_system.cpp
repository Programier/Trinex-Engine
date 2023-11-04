#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <Systems/event_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Window/window.hpp>
#include <Window/window_interface.hpp>
#include <cstring>

#define on_motion_index _M_callbacks_identifier[0]
#define on_button_down_index _M_callbacks_identifier[1]
#define on_button_up_index _M_callbacks_identifier[2]
#define on_wheel_index _M_callbacks_identifier[3]

namespace Engine
{
    MouseSystem* MouseSystem::_M_instance = nullptr;

    MouseSystem& MouseSystem::create()
    {
        Super::create();
        std::memset(&_M_button_status, 0, sizeof(_M_button_status));
        EventSystem* event_system = System::new_system<EventSystem>();
        event_system->register_subsystem(this);

        on_motion_index = event_system->add_listener(
                EventType::MouseMotion, std::bind(&MouseSystem::on_motion_event, this, std::placeholders::_1));
        on_button_up_index = event_system->add_listener(
                EventType::MouseButtonUp, std::bind(&MouseSystem::on_button_up_event, this, std::placeholders::_1));
        on_button_down_index = event_system->add_listener(
                EventType::MouseButtonDown, std::bind(&MouseSystem::on_button_down_event, this, std::placeholders::_1));


        return *this;
    }


    void MouseSystem::on_motion_event(const Event& e)
    {
        const MouseMotionEvent& motion = e.get<const MouseMotionEvent&>();
        _M_pos_info.x                  = motion.x;
        _M_pos_info.y                  = motion.y;
        _M_pos_info.x_offset           = motion.xrel;
        _M_pos_info.y_offset           = motion.yrel;
    }

    void MouseSystem::on_button_down_event(const Event& e)
    {
        const MouseButtonEvent& button_event = e.get<const MouseButtonEvent&>();
        ButtonInfo& info                     = _M_button_status[static_cast<EnumerateType>(button_event.button)];
        info.status                          = Mouse::JustPressed;

        info.clicks = button_event.clicks;
        info.x      = button_event.x;
        info.y      = button_event.y;
    }

    void MouseSystem::on_button_up_event(const Event& e)
    {
        const MouseButtonEvent& button_event = e.get<const MouseButtonEvent&>();
        ButtonInfo& info                     = _M_button_status[static_cast<EnumerateType>(button_event.button)];
        info.status                          = Mouse::JustReleased;

        info.clicks = button_event.clicks;
        info.x      = button_event.x;
        info.y      = button_event.y;
    }

    void MouseSystem::on_wheel_event(const Event& e)
    {}

    MouseSystem& MouseSystem::process_buttons()
    {
        for (auto& info : _M_button_status)
        {
            if (info.status == Mouse::JustPressed)
            {
                info.status = Mouse::Pressed;
            }
            else if (info.status == Mouse::JustReleased)
            {
                info.status = Mouse::Released;
            }

            info.x      = -1;
            info.y      = -1;
            info.clicks = 0;
        }
        return *this;
    }

    MouseSystem& MouseSystem::wait()
    {
        Super::wait();
        return *this;
    }

    MouseSystem& MouseSystem::update(float dt)
    {
        Super::update(dt);
        _M_pos_info.x_offset = 0;
        _M_pos_info.y_offset = 0;

        _M_wheel_info.x = _M_wheel_info.y = 0.0f;
        _M_wheel_info.direction           = Mouse::None;
        return process_buttons();
    }

    MouseSystem& MouseSystem::shutdown()
    {
        Super::shutdown();


        EventSystem::instance()->remove_listener(EventType::MouseMotion, on_motion_index);
        EventSystem::instance()->remove_listener(EventType::MouseButtonUp, on_button_up_index);
        EventSystem::instance()->remove_listener(EventType::MouseButtonDown, on_button_down_index);

        return *this;
    }

    const MouseSystem::PosInfo& MouseSystem::pos_info() const
    {
        return _M_pos_info;
    }

    WindowInterface* window_interface()
    {
        return reinterpret_cast<WindowInterface*>(engine_instance->window()->interface());
    }

    bool MouseSystem::relative_mode() const
    {
        return window_interface()->mouse_relative_mode();
    }

    MouseSystem& MouseSystem::relative_mode(bool flag)
    {
        window_interface()->mouse_relative_mode(flag);
        return *this;
    }

    bool MouseSystem::is_pressed(Mouse::Button button) const
    {
        return _M_button_status[button].status == Mouse::Pressed || is_just_pressed(button);
    }

    bool MouseSystem::is_released(Mouse::Button button) const
    {
        return _M_button_status[button].status == Mouse::Released || is_just_released(button);
    }

    bool MouseSystem::is_just_pressed(Mouse::Button button) const
    {
        return _M_button_status[button].status == Mouse::JustPressed;
    }

    bool MouseSystem::is_just_released(Mouse::Button button) const
    {
        return _M_button_status[button].status == Mouse::JustReleased;
    }

    const MouseSystem::ButtonInfo& MouseSystem::button_info(Mouse::Button button) const
    {
        return _M_button_status[button];
    }

    const MouseSystem::WheelInfo& MouseSystem::wheel_info() const
    {
        return _M_wheel_info;
    }

#define member(x) set(#x, &MouseSystem::x)

    implement_class(MouseSystem, "Engine");

    implement_initialize_class(MouseSystem)
    {}
}// namespace Engine
