#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Systems/system.hpp>
#include <algorithm>
#include <cstring>


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
        throw EngineException("Cannot create new system. Please, call Super::create(); in the overrided method 'create'");
    }

    void System::on_new_system(System* system)
    {
        system->create();
        if (!system->_M_is_fully_created)
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

        name(Strings::format("Engine::Systems::{}", _class->base_name().c_str()), true);
        debug_log("System", "Created system '%s'", string_name().c_str());
        _M_is_fully_created = true;
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

    System* System::find_system_private_no_recurse(const char* _name, size_t len) const
    {
        for (System* system : _M_subsystems)
        {
            const String& system_name = system->string_name();
            if (system_name.length() != len)
                continue;

            if (std::strncmp(_name, system_name.c_str(), len) == 0)
                return system;
        }

        return nullptr;
    }

    System* System::find_subsystem(const char* _name, size_t len)
    {
        const char* end_name       = _name + len;
        const size_t separator_len = Constants::name_separator.length();
        const char* separator      = Strings::strnstr(_name, len, Constants::name_separator.c_str(), separator_len);
        const System* system       = this;


        while (separator && system)
        {
            size_t current_len = separator - _name;
            system             = system->find_system_private_no_recurse(_name, current_len);
            _name              = separator + separator_len;
            separator          = Strings::strnstr(_name, end_name - _name, Constants::name_separator.c_str(), separator_len);
        }

        return system ? system->find_system_private_no_recurse(_name, end_name - _name) : nullptr;
    }

    System* System::find_subsystem(const char* name)
    {
        return find_subsystem(name, std::strlen(name));
    }

    System* System::find_subsystem(const String& name)
    {
        return find_subsystem(name.c_str(), name.length());
    }

    class Class* System::depends_on() const
    {
        return nullptr;
    }


    System& System::shutdown()
    {
        info_log("System", "Shutting down system %p", this);
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

        _M_is_fully_created = false;
        return *this;
    }

    System* System::new_system(class Class* class_instance)
    {
        if (class_instance && class_instance->is_a(System::static_class_instance()))
        {
            System* system = class_instance->create_object()->instance_cast<System>();
            if (system && system->_M_is_fully_created == false)
            {
                on_new_system(system);
            }
            return system;
        }

        return nullptr;
    }

    System* System::new_system(const String& name)
    {
        return new_system(Class::static_find(name));
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
    {
        if (_M_is_fully_created)
        {
            error_log("System", "You must call shutdown method before destroy system! System address %p", this);
        }
    }

    static void bind_to_script(ScriptClassRegistrar* registar, Class* self)
    {
        registar->static_function("System@ new_system(const string& in)", func_of<System*(const String&)>(System::new_system))
                .method("System@ register_subsystem(System@)", &System::register_subsystem)
                .method("System@ remove_subsystem(System@)", &System::remove_subsystem)
                .method("System@ parent_system() const", &System::parent_system)
                .method("System@ sort_subsystems()", &System::sort_subsystems)
                .virtual_method("System@ wait()", func_of<System&(System*)>([](System* self) -> System& { return self->wait(); }))
                .virtual_method("System@ update(float)", func_of<System&(System*, float)>([](System* self, float dt) -> System& {
                                    return self->update(dt);
                                }))
                .virtual_method("System@ shutdown()",
                                func_of<System&(System*)>([](System* self) -> System& { return self->shutdown(); }))
                .virtual_method("Class@ depends_on() const",
                                func_of<Class*(System*)>([](System* self) -> Class* { return self->depends_on(); }));
    }

    implement_class(System, Engine, Class::IsScriptable);
    implement_initialize_class(System)
    {
        static_class_instance()->set_script_registration_callback(bind_to_script);
    }
}// namespace Engine