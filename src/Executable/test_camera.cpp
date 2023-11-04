#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <Graphics/camera.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Window/window.hpp>

namespace Engine
{
    class TestCamera : public Camera
    {
        declare_class(TestCamera, Camera);

    public:
        TestCamera() : Camera({0.0f, 0.0f, 0.0f})
        {
            min_render_distance(0.1);
            max_render_distance(100.f);

            EventSystem* system = EventSystem::instance();

            system->add_listener(Event(EventType::MouseMotion),
                                 std::bind(&TestCamera::on_mouse_move, this, std::placeholders::_1));
        }

        void on_mouse_move(const Event& event)
        {
            if (!MouseSystem::instance()->is_pressed(Mouse::Button::Left))
                return;

            constexpr float sensitivity  = 1.0f;
            const MouseMotionEvent& data = event.get<MouseMotionEvent>();

            Offset2D offset =
                    Offset2D(static_cast<float>(data.yrel) * sensitivity, static_cast<float>(data.xrel) * sensitivity) /
                    Window::instance()->cached_size();

            glm::quat y_rotation = Transform::calc_rotation(transform.right_vector(), offset.x);
            glm::quat x_rotation = Transform::calc_rotation(Constants::OY, -offset.y);
            transform.rotation   = x_rotation * y_rotation * transform.rotation;
        }

        TestCamera& update(float dt) override
        {

            KeyboardSystem* key_system = KeyboardSystem::instance();

            Vector3D velocity     = Vector3D(0.0f);
            constexpr float speed = 0.02;

            if (key_system->is_pressed(Keyboard::W))
            {
                velocity += transform.forward_vector();
            }

            if (key_system->is_pressed(Keyboard::S))
            {
                velocity -= transform.forward_vector();
            }

            if (key_system->is_pressed(Keyboard::D))
            {
                velocity += transform.right_vector();
            }

            if (key_system->is_pressed(Keyboard::A))
            {
                velocity -= transform.right_vector();
            }

            if (key_system->is_pressed(Keyboard::Space))
            {
                if (key_system->is_pressed(Keyboard::LeftShift))
                    velocity -= Constants::OY;
                else
                    velocity += Constants::OY;
            }

            transform.position += velocity * speed;

            Super::update(dt);
            return *this;
        }
    };

    implement_engine_class_default_init(TestCamera);


    Camera* create_test_camera()
    {
        return Object::new_instance_named<TestCamera>("Engine::Test Camera");
    }
}// namespace Engine
