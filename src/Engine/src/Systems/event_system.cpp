#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/keyboard.hpp>
#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Systems/game_controller_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Window/window.hpp>
#include <Window/window_interface.hpp>
#include <api.hpp>


namespace Engine
{
    // Basic callbacks

    static void on_quit(const Event& event)
    {
        engine_instance->request_exit();
    }

    static void on_resize(const Event& event)
    {
        engine_instance->api_interface()->on_window_size_changed();
        Window* window                  = engine_instance->window();
        const WindowEvent& window_event = event.get<const WindowEvent&>();
        Size2D k                        = Size2D(window_event.width, window_event.height) / window->cached_size();

        {
            ViewPort viewport = window->viewport();
            viewport.pos *= k;
            viewport.size *= k;
            window->viewport(viewport);
        }

        {
            Scissor scissor = window->scissor();
            scissor.pos *= k;
            scissor.size *= k;
            window->scissor(scissor);
        }

        window->update_cached_size();
    }

    EventSystem* EventSystem::_M_instance = nullptr;

    EventSystem::EventSystem()
    {}

    const EventSystem& EventSystem::call_listeners(ListenerMap::const_iterator&& it, const Event& event) const
    {
        if (it != _M_listeners.end())
        {
            for (const auto& listener : it->second.callbacks())
            {
                listener.second(event);
            }
        }

        return *this;
    }

    const EventSystem::ListenerMap& EventSystem::listeners() const
    {
        return _M_listeners;
    }

    Identifier EventSystem::add_listener(const Event& event, const Listener& listener)
    {
        return _M_listeners[event.id()].push(listener);
    }

    EventSystem& EventSystem::remove_listener(const Event& event, Identifier id)
    {
        _M_listeners[event.id()].remove(id);
        return *this;
    }


    EventSystem& EventSystem::create()
    {
        EngineSystem::instance()->add_object(this);
        Event event = EventType::Quit;
        add_listener(event, on_quit);
        WindowInterface* interface = reinterpret_cast<WindowInterface*>(engine_instance->window()->interface());
        interface->add_event_callback(id(), [this](const Event& e) { push_event(e); });

        // On resize
        {
            WindowEvent resize_event;
            resize_event.type = WindowEvent::Resized;
            add_listener(Event(EventType::Window, resize_event), on_resize);
        }

        return *this;
    }

    EventSystem& EventSystem::update(float dt)
    {
        Super::update(dt);
        engine_instance->window()->pool_events();
        return *this;
    }

    const EventSystem& EventSystem::push_event(const Event& event) const
    {
        return call_listeners(_M_listeners.find(event.base_id()), event)
                .call_listeners(_M_listeners.find(event.id()), event);
    }

    void EventSystem::init_all()
    {
        new_system<EventSystem>();
        new_system<KeyboardSystem>();
        new_system<MouseSystem>();
        new_system<GameControllerSystem>();
    }


    static Identifier script_add_listener(EventSystem* system, class asIScriptFunction* function)
    {
        ScriptFunction func            = function;
        EventSystem::Listener callback = [func](const Event&) mutable { func.prepare().call(); };
        return system->add_listener(EventType::KeyDown, callback);
    }


    implement_class(EventSystem, "Engine");
    implement_initialize_class(EventSystem)
    {
        Super::initialize_script_bindings<EventSystem>(static_class_instance());
        ScriptClassRegistrar registrar = static_class_instance();

        {
            ScriptEngine::NamespaceSaverScoped sv;
            ScriptEngine::instance()->funcdef("void EventCallback()");
        }

        registrar.method("EventSystem@ instance()", EventSystem::instance);
        registrar.opfunc("uint64 add_listener(EventCallback@)", script_add_listener, ScriptCallConv::CDECL_OBJFIRST);
    }


}// namespace Engine
