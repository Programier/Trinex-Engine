#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/keyboard.hpp>
#include <Core/logger.hpp>
#include <Core/render_thread_call.hpp>
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
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

#include <Window/config.hpp>


namespace Engine
{
    // Basic callbacks

    static void on_window_close(const Event& event)
    {
        WindowManager* manager = WindowManager::instance();
        Window* window         = manager->find(event.window_id());

        if (window)
            manager->destroy_window(window);

        if (manager->windows().empty())
            engine_instance->request_exit();
    }

    static void on_quit(const Event& event)
    {
        WindowManager* manager = WindowManager::instance();
        manager->destroy_window(manager->main_window());
        engine_instance->request_exit();
    }

    static void on_resize(const Event& event)
    {
        WindowManager* manager = WindowManager::instance();
        Window* window         = manager->find(event.window_id());
        if (!window)
            return;

        const WindowEvent& window_event = event.get<const WindowEvent&>();


        {
            auto x                          = window_event.x;
            auto y                          = window_event.y;
            RenderViewport* render_viewport = window->render_viewport();
            if (render_viewport)
            {
                render_viewport->on_resize({x, y});
            }
        }

        Size2D k = Size2D(window_event.width, window_event.height) / window->cached_size();

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
        update_render_targets_size();
    }

    EventSystem::EventSystem()
    {}


    const EventSystem::ListenerMap& EventSystem::listeners() const
    {
        return _M_listeners;
    }

    Identifier EventSystem::add_listener(EventType type, const Listener& listener)
    {
        return _M_listeners[static_cast<byte>(type)].push(listener);
    }

    EventSystem& EventSystem::remove_listener(EventType type, Identifier id)
    {
        _M_listeners[static_cast<byte>(type)].remove(id);
        return *this;
    }


    EventSystem& EventSystem::create()
    {
        Super::create();

        System::new_system<EngineSystem>()->register_subsystem(this);
        add_listener(EventType::Quit, on_quit);
        add_listener(EventType::WindowClose, on_window_close);

        WindowManager::instance()->add_event_callback(id(), [this](const Event& e) { push_event(e); });

        add_listener(EventType::WindowResized, on_resize);
        add_listener(EventType::WindowSizeChanged, on_resize);

        // Register subsystems
        new_system<KeyboardSystem>();
        new_system<MouseSystem>();
        new_system<GameControllerSystem>();

        return *this;
    }

    EventSystem& EventSystem::update(float dt)
    {
        Super::update(dt);
        WindowManager::instance()->pool_events();


        if (KeyboardSystem::instance()->is_just_pressed(Keyboard::Key::G))
        {
            WindowManager::instance()->create_window(global_window_config, nullptr);
        }
        return *this;
    }

    const EventSystem& EventSystem::push_event(const Event& event) const
    {
        auto it = _M_listeners.find(static_cast<byte>(event.type()));
        if (it != _M_listeners.end())
        {
            it->second.trigger(event);
        }

        return *this;
    }

    implement_class(EventSystem, "Engine", 0);
    implement_initialize_class(EventSystem)
    {}


}// namespace Engine
