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
                .method(Strings::format("const {}& {}() const", name, func).c_str(),
                        func_of<const Type&(const Event*)>(
                                [](const Event* event) -> const Type& { return event->get<const Type&>(); }),
                        ScriptCallConv::CDECL_OBJFIRST);

        register_event_data_props(registrar, args...);
    }

    static void on_init()
    {}

    static ReflectionInitializeController initializer(on_init, "Engine::EventData",
                                                      {"Engine::Event", "Engine::Keyboard", "Engine::GameController"});
}// namespace Engine
