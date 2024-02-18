#include <Core/engine_loading_controllers.hpp>
#include <Event/listener_id.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{

    EventSystemListenerID::EventSystemListenerID() : _M_type(EventType::Undefined), _M_id(0)
    {}

    EventSystemListenerID::EventSystemListenerID(EventType type, Identifier id) : _M_type(type), _M_id(id)
    {}

    default_copy_constructors_cpp(EventSystemListenerID);

    static void bind_event_system_listener_id()
    {
        ScriptEngineInitializeController().require("Bind Event");

        ScriptClassRegistrar::ClassInfo info = ScriptClassRegistrar::create_type_info<EventSystemListenerID>(
                ScriptClassRegistrar::Value | ScriptClassRegistrar::AppClassAllInts | ScriptClassRegistrar::POD);

        ScriptClassRegistrar registrar("Engine::EventSystemListenerID", info);
        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<EventSystemListenerID>);

        registrar.behave(ScriptClassBehave::Construct, "void f(Engine::EventType, uint64)",
                         ScriptClassRegistrar::constructor<EventSystemListenerID, EventType, Identifier>);
        registrar.behave(ScriptClassBehave::Construct, "void f(const EventSystemListenerID& in)",
                         ScriptClassRegistrar::constructor<EventSystemListenerID, const EventSystemListenerID&>);
        registrar.method("EventSystemListenerID& opAssign(const EventSystemListenerID& in)",
                         method_of<EventSystemListenerID&, EventSystemListenerID, const EventSystemListenerID&>(
                                 &EventSystemListenerID::operator=));
    }

    static ScriptEngineInitializeController on_script_init(bind_event_system_listener_id, "Bind EventSystenListenerID");
}// namespace Engine
