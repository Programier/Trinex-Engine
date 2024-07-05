#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_primitives.hpp>

#define declare_primitive(name, type)                                                                                            \
    name::name(type initial) : value(initial)                                                                                    \
    {}                                                                                                                           \
    name::name(const name&)                  = default;                                                                          \
    name& name::operator=(const name& other) = default;                                                                          \
    void name::add_ref() const                                                                                                   \
    {                                                                                                                            \
        ++m_refs;                                                                                                                \
    }                                                                                                                            \
    void name::release() const                                                                                                   \
    {                                                                                                                            \
        --m_refs;                                                                                                                \
        if (m_refs == 0)                                                                                                         \
        {                                                                                                                        \
            delete this;                                                                                                         \
        }                                                                                                                        \
    }                                                                                                                            \
    type name::op_conv() const                                                                                                   \
    {                                                                                                                            \
        return value;                                                                                                            \
    }

declare_primitive(Boolean, bool);
declare_primitive(Integer8, Engine::int8_t);
declare_primitive(Integer16, Engine::int16_t);
declare_primitive(Integer32, Engine::int32_t);
declare_primitive(Integer64, Engine::int64_t);
declare_primitive(UnsignedInteger8, Engine::uint8_t);
declare_primitive(UnsignedInteger16, Engine::uint16_t);
declare_primitive(UnsignedInteger32, Engine::uint32_t);
declare_primitive(UnsignedInteger64, Engine::uint64_t);
declare_primitive(Float, float);
declare_primitive(Double, double);

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
        register_base_type<Boolean, bool>("Bool", "bool", "false");
        register_base_type<Integer8, Engine::int8_t>("Int8", "int8", "0");
        register_base_type<Integer16, Engine::int16_t>("Int16", "int16", "0");
        register_base_type<Integer32, Engine::int32_t>("Int32", "int32", "0");
        register_base_type<Integer64, Engine::int64_t>("Int64", "int64", "0");
        register_base_type<UnsignedInteger8, Engine::uint8_t>("UInt8", "uint8", "0");
        register_base_type<UnsignedInteger16, Engine::uint16_t>("UInt16", "uint16", "0");
        register_base_type<UnsignedInteger32, Engine::uint32_t>("UInt32", "uint32", "0");
        register_base_type<UnsignedInteger64, Engine::uint64_t>("UInt64", "uint64", "0");
        register_base_type<Float, float>("Float", "float", "0.0");
        register_base_type<Double, double>("Double", "double", "0.0");
    }
}// namespace Engine::Initializers
