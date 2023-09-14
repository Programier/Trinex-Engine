#include <Core/class.hpp>
#include <Core/game_controller.hpp>
#include <Event/event_data.hpp>
#include <Systems/event_system.hpp>
#include <Systems/game_controller_system.hpp>

namespace Engine
{
    implement_class(GameControllerSystem, "Engine");
    implement_initialize_class(GameControllerSystem)
    {}

#define new_id(x) _M_callbacks_id.push_back(x)

    GameControllerSystem* GameControllerSystem::_M_instance = nullptr;


    void GameControllerSystem::on_controller_added(const Event& event)
    {
        const ControllerDeviceAddedEvent& e = event.get<const ControllerDeviceAddedEvent&>();
        _M_controllers[e.id]                = new GameController(e.id);
    }

    void GameControllerSystem::on_controller_removed(const Event& event)
    {
        const ControllerDeviceRemovedEvent& e = event.get<const ControllerDeviceRemovedEvent&>();
        delete _M_controllers[e.id];
        _M_controllers.erase(e.id);
    }

    void GameControllerSystem::on_axis_motion(const Event& event)
    {
        try
        {
            const ControllerAxisMotionEvent& e              = event.get<const ControllerAxisMotionEvent&>();
            _M_controllers.at(e.id)->_M_axis_values[e.axis] = e.value;
        }
        catch (...)
        {}
    }


    GameControllerSystem::GameControllerSystem()
    {}

    GameControllerSystem& GameControllerSystem::create()
    {
        Super::create();

        EventSystem* event_system = System::new_system<EventSystem>();

        new_id(event_system->add_listener(
                EventType::ControllerDeviceAdded,
                std::bind(&GameControllerSystem::on_controller_added, this, std::placeholders::_1)));

        new_id(event_system->add_listener(
                EventType::ControllerDeviceRemoved,
                std::bind(&GameControllerSystem::on_controller_removed, this, std::placeholders::_1)));

        new_id(event_system->add_listener(
                EventType::ControllerAxisMotion,
                std::bind(&GameControllerSystem::on_axis_motion, this, std::placeholders::_1)));

        return *this;
    }

    void GameControllerSystem::wait()
    {
        Super::wait();
    }

    GameControllerSystem& GameControllerSystem::update(float dt)
    {
        Super::update(dt);
        return *this;
    }

    GameControllerSystem& GameControllerSystem::shutdown()
    {
        Super::shutdown();
        return *this;
    }

    GameController* GameControllerSystem::controller(Identifier id) const
    {
        auto it = _M_controllers.find(id);
        if (it != _M_controllers.end())
        {
            return it->second;
        }

        return nullptr;
    }

}// namespace Engine
