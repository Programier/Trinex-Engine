#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Event/listener_id.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{

    EventSystemListenerID::EventSystemListenerID() : m_type(EventType::Undefined), m_id(0)
    {}

    EventSystemListenerID::EventSystemListenerID(EventType type, Identifier id) : m_type(type), m_id(id)
    {}

    default_copy_constructors_cpp(EventSystemListenerID);

    static void bind_event_system_listener_id()
    {
        ScriptClassRegistrar::ClassInfo info = ScriptClassRegistrar::create_type_info<EventSystemListenerID>(
                ScriptClassRegistrar::Value | ScriptClassRegistrar::AppClassAllInts | ScriptClassRegistrar::POD);

        ScriptClassRegistrar registrar("Engine::EventSystemListenerID", info);
        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<EventSystemListenerID>);

        registrar.behave(ScriptClassBehave::Construct, "void f(Engine::EventType, uint64)",
                         ScriptClassRegistrar::constructor<EventSystemListenerID, EventType, Identifier>);
        registrar.behave(ScriptClassBehave::Construct, "void f(const EventSystemListenerID& in)",
                         ScriptClassRegistrar::constructor<EventSystemListenerID, const EventSystemListenerID&>);
        registrar.method("EventSystemListenerID& opAssign(const EventSystemListenerID& in)",
                         method_of<EventSystemListenerID&, const EventSystemListenerID&>(&EventSystemListenerID::operator=));
    }

    static ReflectionInitializeController on_script_init(bind_event_system_listener_id, "Engine::EventSystenListenerID",
                                                         {"Engine::Event"});
}// namespace Engine
