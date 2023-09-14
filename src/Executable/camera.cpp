#include <Graphics/camera.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>

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
            Vector2D offset = {pos_info.x_offset, pos_info.y_offset};

            offset.x /= 1280;
            offset.y /= 720;
            camera->transform.rotate(-offset.x, Engine::Constants::OY, true);
            camera->transform.rotate(offset.y, camera->transform.right_vector(), true);
        }
    }
}// namespace Engine
