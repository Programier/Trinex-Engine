#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
    static void register_event_data_props(ScriptClassRegistrar& registrar)
    {}

    template<typename Address, typename... Args>
    void register_event_data_props(ScriptClassRegistrar& registrar, const char* name, Address address, Args... args)
    {
        registrar.property(name, address);
        register_event_data_props(registrar, args...);
    }

    template<typename Type, typename... Args>
    static void register_event_data_type(const char* name, const char* func, Args... args)
    {
        ScriptClassRegistrar registrar(name, ScriptClassRegistrar::create_type_info<Type>(ScriptClassRegistrar::Value));
        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Type>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, Strings::format("void f(const {}& in)", name).c_str(),
                         ScriptClassRegistrar::constructor<Type, const Type&>, ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Type>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.method(Strings::format("{}& opAssign(const {}& in)", name, name).c_str(),
                         method_of<Type&, Type, const Type&>(&Type::operator=));

        ScriptClassRegistrar("Engine::Event")
                .func_as_method(Strings::format("const {}& {}() const", name, func).c_str(),
                                func_of<const Type&(const Event*)>(
                                        [](const Event* event) -> const Type& { return event->get<const Type&>(); }),
                                ScriptCallConv::CDECL_OBJFIRST);

        register_event_data_props(registrar, args...);
    }

    static void on_init()
    {
        register_event_data_type<DisplayEvent>("Engine::DisplayEvent", "display_event");
        register_event_data_type<WindowEvent>("Engine::WindowEvent", "window_event", "int x", &WindowEvent::x, "int y",
                                              &WindowEvent::y, "int width", &WindowEvent::width, "int height",
                                              &WindowEvent::height);
        register_event_data_type<KeyEvent>("Engine::KeyEvent", "key_event", "Engine::Keyboard::Key key", &KeyEvent::key,
                                           "bool repeat", &KeyEvent::repeat);
        register_event_data_type<MouseMotionEvent>("Engine::MouseMotionEvent", "mouse_motion_event", "int x",
                                                   &MouseMotionEvent::x, "int y", &MouseMotionEvent::y, "int xrel",
                                                   &MouseMotionEvent::xrel, "int yrel", &MouseMotionEvent::yrel);
        register_event_data_type<ControllerDeviceAddedEvent>("Engine::ControllerDeviceAddedEvent",
                                                             "controller_device_added_event", "uint64 id",
                                                             &ControllerDeviceAddedEvent::id);
        register_event_data_type<ControllerDeviceRemovedEvent>("Engine::ControllerDeviceRemovedEvent",
                                                               "controller_device_removed_event", "uint64 id",
                                                               &ControllerDeviceRemovedEvent::id);
        register_event_data_type<MouseButtonEvent>(
                "Engine::MouseButtonEvent", "mouse_button_event", "Engine::Mouse::Button button", &MouseButtonEvent::button,
                "uint clicks", &MouseButtonEvent::clicks, "int x", &MouseButtonEvent::x, "int y", &MouseButtonEvent::y);
        register_event_data_type<MouseWheelEvent>("Engine::MouseWheelEvent", "mouse_wheel_event",
                                                  "Engine::Mouse::Direction direction", &MouseWheelEvent::direction, "float x",
                                                  &MouseWheelEvent::x, "float y", &MouseWheelEvent::y);
        register_event_data_type<ControllerAxisMotionEvent>(
                "Engine::ControllerAxisMotionEvent", "controller_axis_motion_event", "uint64 id", &ControllerAxisMotionEvent::id,
                "int16 value", &ControllerAxisMotionEvent::value, "Engine::GameController::Axis axis",
                &ControllerAxisMotionEvent::axis);
    }

    static ScriptEngineInitializeController initializer(on_init, "Bind EventData",
                                                        {"Bind Event", "Bind Keyboard", "Bind Mouse", "Bind GameController"});
}// namespace Engine
