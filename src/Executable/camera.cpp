#include <Core/engine.hpp>
#include <Event/event_data.hpp>
#include <Graphics/camera.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Window/window.hpp>

namespace Engine
{
    float speed              = 5.0;
    float current_diff       = 0;
    float K                  = 0.5;
    bool using_relative_mode = false;


    void enable_relative_mode(const Event& e)
    {
        MouseSystem::instance()->relative_mode(true);
        using_relative_mode = true;
    }

    void disable_relative_mode(const Event& e)
    {
        MouseSystem::instance()->relative_mode(false);
        using_relative_mode = false;
    }

    void init_camera()
    {
        // Button press event
        MouseButtonEvent b_event;
        b_event.type = Mouse::Button::X2;

        EventSystem::instance()->add_listener(Event(EventType::MouseButtonDown, b_event), enable_relative_mode);
        b_event.type = Mouse::Button::X1;
        EventSystem::instance()->add_listener(Event(EventType::MouseButtonUp, b_event), disable_relative_mode);
    }

    void update_camera(Camera* camera, float dt)
    {
        KeyboardSystem* keyboard_system = KeyboardSystem::instance();
        current_diff                    = (current_diff * K) + (dt * (1.0 - K));
        float current_speed             = speed * current_diff;


        if (keyboard_system->is_pressed(Keyboard::Key::W))
        {
            camera->transform.move(camera->transform.front_vector() * current_speed, true);
        }

        if (keyboard_system->is_pressed(Keyboard::Key::S))
        {
            camera->transform.move(camera->transform.front_vector() * -current_speed, true);
        }

        if (keyboard_system->is_pressed(Keyboard::Key::D))
        {
            camera->transform.move(camera->transform.right_vector() * current_speed, true);
        }

        if (keyboard_system->is_pressed(Keyboard::Key::A))
        {
            camera->transform.move(camera->transform.right_vector() * -current_speed, true);
        }

        if (keyboard_system->is_pressed(Keyboard::Key::Space))
        {
            float k = (keyboard_system->is_pressed(Keyboard::Key::LeftShift) ? -1 : 1);
            camera->transform.move(camera->transform.up_vector() * current_speed * k, true);
        }

        if (using_relative_mode)
        {
            auto& pos_info  = MouseSystem::instance()->pos_info();
            Vector2D offset = {pos_info.x_offset * dt / 5.f, pos_info.y_offset * dt / 5.f};
            camera->transform.rotate(-offset.x, Engine::Constants::OY, true);
            camera->transform.rotate(offset.y, camera->transform.right_vector(), true);
        }
    }
}// namespace Engine
