#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/enum.hpp>
#include <Core/keyboard.hpp>
#include <Core/logger.hpp>
#include <Core/threading.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Systems/game_controller_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <angelscript.h>


namespace Engine
{
    // Basic callbacks

    void EventSystem::on_window_close(const Event& event, bool is_quit)
    {
        WindowManager* manager = WindowManager::instance();
        Window* window         = manager->find(event.window_id());

        if (manager->main_window() == window || is_quit)
        {
            engine_instance->request_exit();
        }
        else if (window)
        {
            m_windows_to_destroy.push_back(window);
        }
    }

    static void on_resize(const Event& event)
    {
        WindowManager* manager = WindowManager::instance();
        Window* window         = manager->find(event.window_id());
        if (!window)
            return;

        const WindowEvent& window_event = event.get<const WindowEvent&>();
        window->update_cached_size();

        {
            auto x                          = window_event.x;
            auto y                          = window_event.y;
            RenderViewport* render_viewport = window->render_viewport();
            if (render_viewport)
            {
                render_viewport->on_resize({x, y});
            }
        }
    }

    EventSystem::EventSystem()
    {}


    const EventSystem::ListenerMap& EventSystem::listeners() const
    {
        return m_listeners;
    }

    EventSystemListenerID EventSystem::add_listener(EventType type, const Listener& listener)
    {
        return EventSystemListenerID(type, m_listeners[static_cast<byte>(type)].push(listener));
    }

    EventSystem& EventSystem::remove_listener(const EventSystemListenerID& id)
    {
        m_listeners_to_remove.push_back(id);
        return *this;
    }

    EventSystem& EventSystem::create()
    {
        Super::create();

        System::new_system<EngineSystem>()->register_subsystem(this);
        add_listener(EventType::Quit, std::bind(&EventSystem::on_window_close, this, std::placeholders::_1, true));
        add_listener(EventType::WindowClose, std::bind(&EventSystem::on_window_close, this, std::placeholders::_1, true));
        add_listener(EventType::WindowResized, on_resize);

        // Register subsystems
        new_system<KeyboardSystem>();
        new_system<MouseSystem>();
        new_system<GameControllerSystem>();

        process_event_method(ProcessEventMethod::PoolEvents);

        return *this;
    }

    EventSystem& EventSystem::update(float dt)
    {
        Super::update(dt);

        if (!m_listeners_to_remove.empty())
        {
            for (auto& id : m_listeners_to_remove)
            {
                m_listeners[static_cast<byte>(id.m_type)].remove(id.m_id);
            }
            m_listeners_to_remove.clear();
        }


        (this->*m_process_events)();

        if (!m_windows_to_destroy.empty())
        {
            WindowManager* manager = WindowManager::instance();

            if (manager)
            {
                for (Window* window : m_windows_to_destroy)
                {
                    manager->destroy_window(window);
                }
            }

            m_windows_to_destroy.clear();
        }

        return *this;
    }

    const EventSystem& EventSystem::push_event(const Event& event) const
    {
        auto it = m_listeners.find(static_cast<byte>(event.type()));
        if (it != m_listeners.end())
        {
            it->second.trigger(event);
        }

        return *this;
    }

    EventSystem& EventSystem::shutdown()
    {
        Super::shutdown();
        m_listeners.clear();
        return *this;
    }

    Name EventSystem::event_name(EventType type)
    {
        static Enum* event_type_enum = Enum::static_find("Engine::EventType", true);
        auto entry                   = event_type_enum->entry(static_cast<EnumerateType>(type));

        if (entry)
        {
            return entry->name;
        }

        return Name::undefined;
    }

    EventSystem& EventSystem::wait_events()
    {
        WindowManager::instance()->wait_for_events();
        return *this;
    }

    EventSystem& EventSystem::pool_events()
    {
        WindowManager::instance()->pool_events();
        return *this;
    }

    EventSystem& EventSystem::process_event_method(ProcessEventMethod method)
    {
        if (method == ProcessEventMethod::PoolEvents)
        {
            m_process_events = &EventSystem::pool_events;
        }
        else
        {
            m_process_events = &EventSystem::wait_events;
        }
        return *this;
    }


    static EventSystemListenerID add_script_listener(EventSystem* system, EventType type, asIScriptFunction* _function)
    {
        ScriptFunction function        = _function;
        EventSystem::Listener callback = [function](const Event& event) mutable {
            const void* address = &event;
            //function.prepare().arg_address(0, const_cast<void*>(address)).call().unbind_context();
        };
        return system->add_listener(type, callback);
    }


    static void init_script_class(ScriptClassRegistrar* registrar, Class* self)
    {
        ReflectionInitializeController()
                .require("Engine::Event")
                .require("Engine::EventSystenListenerID")
                .require("Engine::EventType");

        ScriptEnumRegistrar enum_regisrar("Engine::EventSystem::ProcessEventMethod");
        enum_regisrar.set("PoolEvents", EventSystem::PoolEvents);
        enum_regisrar.set("WaitingEvents", EventSystem::WaitingEvents);


        // ScriptEngine::instance()->funcdef("void Engine::EventCallback(const Engine::Event&)");


        // registrar->static_function("EventSystem@ instance()", EventSystem::new_system<EventSystem>);
        // registrar->func_as_method("uint64 add_listener(Engine::EventType, Engine::EventCallback@)", add_script_listener,
        //                           ScriptCallConv::CDECL_OBJFIRST);
        // registrar->method("EventSystem& remove_listener(const Engine::EventType, uint64 id)", &EventSystem::remove_listener);
        // registrar->method("const EventSystem& push_event(const Engine::Event& in) const", &EventSystem::push_event);
        // registrar->method("EventSystem& process_event_method(Engine::EventSystem::ProcessEventMethod)",
        //                   &EventSystem::process_event_method);
    }

    implement_engine_class(EventSystem, Class::IsScriptable)
    {
        static_class_instance()->set_script_registration_callback(init_script_class);
    }


}// namespace Engine
