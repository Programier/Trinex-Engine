#include <Core/string_functions.hpp>
#include <ScriptEngine/primitive_wrappers.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <iostream>

#define declare_primitive(name, type)                                                                                  \
    name::name(type initial) : value(initial)                                                                          \
    {}                                                                                                                 \
    name::name(const name&)                  = default;                                                                \
    name& name::operator=(const name& other) = default;                                                                \
    void name::add_ref() const                                                                                         \
    {                                                                                                                  \
        ++_M_refs;                                                                                                     \
    }                                                                                                                  \
    void name::release() const                                                                                         \
    {                                                                                                                  \
        --_M_refs;                                                                                                     \
        if (_M_refs == 0)                                                                                              \
        {                                                                                                              \
            delete this;                                                                                               \
        }                                                                                                              \
    }                                                                                                                  \
    type name::op_conv() const                                                                                         \
    {                                                                                                                  \
        return value;                                                                                                  \
    }


declare_primitive(Boolean, bool);
declare_primitive(Integer8, Engine::int8_t);
declare_primitive(Integer16, Engine::int16_t);
declare_primitive(Integer, Engine::int32_t);
declare_primitive(Integer64, Engine::int64_t);
declare_primitive(UnsignedInteger8, Engine::uint8_t);
declare_primitive(UnsignedInteger16, Engine::uint16_t);
declare_primitive(UnsignedInteger, Engine::uint32_t);
declare_primitive(UnsignedInteger64, Engine::uint64_t);
declare_primitive(Float, float);

namespace Engine::Initializers
{
    template<typename T1, typename T2>
    T1* factory(T2 value)
    {
        T1* instance = new T1(value);
        instance->add_ref();
        return instance;
    }


    template<typename T, typename BaseType>
    void register_base_type(const char* name, const char* base, const char* default_v)
    {
        ScriptClassRegistrar::ClassInfo info;
        info.size  = sizeof(T);
        info.flags = ScriptClassRegistrar::Ref;

        ScriptClassRegistrar registrar(name, info);

        registrar.behave(ScriptClassBehave::Factory, Strings::format("{}@ f({} = {})", name, base, default_v).c_str(),
                         factory<T, BaseType>, ScriptCallConv::CDECL);
        registrar.behave(ScriptClassBehave::AddRef, "void AddRef()", &T::add_ref, ScriptCallConv::THISCALL);
        registrar.behave(ScriptClassBehave::Release, "void Release()", &T::release, ScriptCallConv::THISCALL);
        registrar.method(Strings::format("{} opConv() const", base).c_str(), &T::op_conv);
        registrar.property(Strings::format("{} value", base).c_str(), &T::value);
    }

    void init_primitive_wrappers()
    {
        register_base_type<Boolean, bool>("Boolean", "bool", "false");
        register_base_type<Integer8, Engine::int8_t>("Integer8", "int8", "0");
        register_base_type<Integer16, Engine::int16_t>("Integer16", "int16", "0");
        register_base_type<Integer, Engine::int32_t>("Integer", "int32", "0");
        register_base_type<Integer64, Engine::int64_t>("Integer64", "int64", "0");
        register_base_type<UnsignedInteger8, Engine::uint8_t>("UnsignedInteger8", "uint8", "0");
        register_base_type<UnsignedInteger16, Engine::uint16_t>("UnsignedInteger16", "uint16", "0");
        register_base_type<UnsignedInteger, Engine::uint32_t>("UnsignedInteger", "uint32", "0");
        register_base_type<UnsignedInteger64, Engine::uint64_t>("UnsignedInteger64", "uint64", "0");
    }
}// namespace Engine::PrimitiveWrappers
