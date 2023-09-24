#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>


namespace Engine
{

    static bool system_filter(Object* object)
    {
        return object && object->is_instance_of<System>();
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

        //        if (!_class->has_flag(Class::IsSingletone))
        //        {
        //            throw EngineException("Each class based from Engine::System must be singletone!");
        //        }

        name(_class->name() + " [System]");
        add_filter(system_filter);

        debug_log("System", "Created system '%s'", string_name().c_str());
        return *this;
    }

    System& System::update(float dt)
    {
        for (Object* system : objects())
        {
            reinterpret_cast<System*>(system)->update(dt);
        }
        return *this;
    }

    void System::wait()
    {}

    System& System::shutdown()
    {
        System* parent_system = package()->instance_cast<System>();
        if (parent_system)
        {
            parent_system->remove_object(this);
        }

        // Shutdown child systems

        const Vector<Object*>& subsystems = objects();
        while (!subsystems.empty())
        {
            System* subsystem = reinterpret_cast<System*>(subsystems.front());
            subsystem->shutdown();
            remove_object(subsystem);
        }

        return *this;
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
