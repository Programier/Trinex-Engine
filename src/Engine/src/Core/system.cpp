#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>
#include <algorithm>


namespace Engine
{

    System::UpdateTask::UpdateTask(System* system, float dt) : _M_system(system), _M_dt(dt)
    {}

    int_t System::UpdateTask::execute()
    {
        _M_system->update(_M_dt);
        return sizeof(System::UpdateTask);
    }

    void System::on_create_fail()
    {
        throw EngineException(
                "Cannot create new system. Please, call Super::create(); in the overrided method 'create'");
    }

    void System::on_new_system(System* system)
    {
        system->create();
        if (!system->is_fully_created)
        {
            on_create_fail();
        }
    }

    System::System() : _M_parent_system(nullptr)
    {}

    System& System::create()
    {
        const Class* _this  = This::static_class_instance();
        const Class* _class = class_instance();

        if (_class == nullptr || _this == nullptr || _this == _class)
        {
            throw EngineException("Each class based from Engine::System must be registered!");
        }

        name(Strings::format("Engine::Systems::{}", _class->base_name()));
        debug_log("System", "Created system '%s'", string_name().c_str());
        is_fully_created = true;
        return *this;
    }

    System& System::update(float dt)
    {
        for (System* system : _M_subsystems)
        {
            system->update(dt);
        }
        return *this;
    }

    System& System::wait()
    {
        for (System* subsystem : _M_subsystems)
        {
            subsystem->wait();
        }

        return *this;
    }

    System& System::register_subsystem(System* system)
    {
        if (system->parent_system() == this)
            return *this;

        if (system->_M_parent_system)
        {
            system->_M_parent_system->remove_subsystem(system);
        }

        _M_subsystems.push_back(system);
        system->_M_parent_system = this;

        return *this;
    }

    System& System::remove_subsystem(System* system)
    {
        if (system->_M_parent_system == this)
        {
            auto it  = _M_subsystems.begin();
            auto end = _M_subsystems.end();

            while (it != end)
            {
                if (*it == system)
                {
                    _M_subsystems.erase(it);
                    system->_M_parent_system = nullptr;
                    return *this;
                }
                ++it;
            }
        }
        return *this;
    }

    System* System::parent_system() const
    {
        return _M_parent_system;
    }


    static bool sort_systems_predicate(System* first, System* second)
    {
        Class* _first  = first->class_instance();
        Class* _second = second->depends_on();


        while (_first && _second)
        {
            if (_first == _second)
                return true;

            System* system = _second->singletone_instance()->instance_cast<System>();

            if (system)
            {
                _second = system->depends_on();
            }
            else
            {
                _second = nullptr;
            }
        }

        return false;
    }

    System& System::sort_subsystems()
    {
        std::sort(_M_subsystems.begin(), _M_subsystems.end(), sort_systems_predicate);
        std::for_each(_M_subsystems.begin(), _M_subsystems.end(), [](System* system) { system->sort_subsystems(); });
        return *this;
    }

    class Class* System::depends_on() const
    {
        return nullptr;
    }


    System& System::shutdown()
    {
        if (_M_parent_system)
        {
            _M_parent_system->remove_subsystem(this);
        }

        // Shutdown child systems

        Vector<System*> subsystems = std::move(_M_subsystems);
        for (System* system : subsystems)
        {
            system->shutdown();
        }

        return *this;
    }

    System* System::new_system_by_name(const String& name)
    {
        Class* class_instance = Class::static_find_class(name);
        if (class_instance && class_instance->contains_class(System::static_class_instance()))
        {
            System* system = class_instance->create_object()->instance_cast<System>();
            if (system && system->is_fully_created == false)
            {
                on_new_system(system);
            }
            return system;
        }
        return nullptr;
    }

    const Vector<System*>& System::subsystems() const
    {
        return _M_subsystems;
    }

    Identifier System::id() const
    {
        return reinterpret_cast<Identifier>(this);
    }

    System::~System()
    {}

    implement_class(System, "Engine", 0);
    implement_default_initialize_class(System);
}// namespace Engine
