#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>
#include <algorithm>


namespace Engine
{

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

    System::System()
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
        for(System* subsystem : _M_subsystems)
        {
            subsystem->wait();
        }

        return *this;
    }


    System& System::on_child_remove(Object* object)
    {
        System* system = object->instance_cast<System>();
        if (system)
        {
            return remove_subsystem(system);
        }

        return *this;
    }

    System& System::on_child_set(Object* object)
    {
        return *this;
    }


    System& System::register_subsystem(System* system, Index index)
    {
        if (system->owner() == this)
            return *this;

        system->owner(this);

        if (index >= _M_subsystems.size())
        {
            _M_subsystems.push_back(system);
        }
        else
        {
            _M_subsystems.insert(_M_subsystems.begin() + index, system);
        }

        return *this;
    }

    System& System::remove_subsystem(System* system)
    {
        auto it = _M_subsystems.begin();
        auto end = _M_subsystems.end();

        while(it != end)
        {
            if(*it == system)
            {
                _M_subsystems.erase(it);
                return *this;
            }
            ++it;
        }

        return *this;
    }


    System& System::shutdown()
    {
        owner(nullptr);

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

    implement_class(System, "Engine");
    implement_default_initialize_class(System);
}// namespace Engine
