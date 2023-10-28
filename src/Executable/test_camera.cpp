#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <Graphics/camera.hpp>
#include <Systems/event_system.hpp>

namespace Engine
{
    class TestCamera : public Camera
    {
        declare_class(TestCamera, Camera);

    public:
        TestCamera()
        {
            EventSystem* system = EventSystem::instance();

            KeyEvent key_event;
            key_event.key    = Keyboard::W;

            system->add_listener(Event(EventType::KeyDown, key_event),
                                 std::bind(&TestCamera::on_w_press, this, std::placeholders::_1));
            system->add_listener(Event(EventType::KeyUp, key_event),
                                 std::bind(&TestCamera::on_w_release, this, std::placeholders::_1));

            key_event.key = Keyboard::S;

            system->add_listener(Event(EventType::KeyDown, key_event),
                                 std::bind(&TestCamera::on_s_press, this, std::placeholders::_1));
            system->add_listener(Event(EventType::KeyUp, key_event),
                                 std::bind(&TestCamera::on_s_release, this, std::placeholders::_1));

            key_event.key = Keyboard::A;

            system->add_listener(Event(EventType::KeyDown, key_event),
                                 std::bind(&TestCamera::on_a_press, this, std::placeholders::_1));
            system->add_listener(Event(EventType::KeyUp, key_event),
                                 std::bind(&TestCamera::on_a_release, this, std::placeholders::_1));

            key_event.key = Keyboard::D;

            system->add_listener(Event(EventType::KeyDown, key_event),
                                 std::bind(&TestCamera::on_d_press, this, std::placeholders::_1));
            system->add_listener(Event(EventType::KeyUp, key_event),
                                 std::bind(&TestCamera::on_d_release, this, std::placeholders::_1));
        }

        void on_w_press(const Event& event)
        {
            info_log("TestCamera", "%s", __PRETTY_FUNCTION__);
        }

        void on_w_release(const Event& event)
        {
            info_log("TestCamera", "%s", __PRETTY_FUNCTION__);
        }

        void on_s_press(const Event& event)
        {
            info_log("TestCamera", "%s", __PRETTY_FUNCTION__);
        }

        void on_s_release(const Event& event)
        {
            info_log("TestCamera", "%s", __PRETTY_FUNCTION__);
        }

        void on_a_press(const Event& event)
        {
            info_log("TestCamera", "%s", __PRETTY_FUNCTION__);
        }

        void on_a_release(const Event& event)
        {
            info_log("TestCamera", "%s", __PRETTY_FUNCTION__);
        }

        void on_d_press(const Event& event)
        {
            info_log("TestCamera", "%s", __PRETTY_FUNCTION__);
        }

        void on_d_release(const Event& event)
        {
            info_log("TestCamera", "%s", __PRETTY_FUNCTION__);
        }

        TestCamera& update(float dt) override
        {
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
