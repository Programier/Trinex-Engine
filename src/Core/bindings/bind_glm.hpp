#pragma once
#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
    namespace fmt = Strings;

    // Helpers
    template<typename Type>
    inline ScriptClassRegistrar::ClassInfo info_of()
    {
        return ScriptClassRegistrar::create_type_info<Type>(ScriptClassRegistrar::ClassFlags::Value);
    }

    // We need to implement wrappers for glm::vec* objects


#define implement_vector_wrapper(name)                                                                                           \
    struct name##Wrapper : public name {                                                                                         \
        name& obj()                                                                                                              \
        {                                                                                                                        \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        const name& obj() const                                                                                                  \
        {                                                                                                                        \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper()                                                                                                          \
        {}                                                                                                                       \
                                                                                                                                 \
        template<typename... Args>                                                                                               \
        name##Wrapper(Args... args) : name(args...)                                                                              \
        {}                                                                                                                       \
                                                                                                                                 \
        name##Wrapper(const name##Wrapper& vec)                                                                                  \
        {                                                                                                                        \
            obj() = vec.obj();                                                                                                   \
        }                                                                                                                        \
                                                                                                                                 \
        bool operator==(const name##Wrapper& wrap) const                                                                         \
        {                                                                                                                        \
            return obj() == wrap.obj();                                                                                          \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator=(const name##Wrapper& new_obj)                                                                   \
        {                                                                                                                        \
            obj() = new_obj.obj();                                                                                               \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator+=(const name##Wrapper& new_obj)                                                                  \
        {                                                                                                                        \
            obj() += new_obj.obj();                                                                                              \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator-=(const name##Wrapper& new_obj)                                                                  \
        {                                                                                                                        \
            obj() -= new_obj.obj();                                                                                              \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator/=(const name##Wrapper& new_obj)                                                                  \
        {                                                                                                                        \
            obj() /= new_obj.obj();                                                                                              \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator*=(const name##Wrapper& new_obj)                                                                  \
        {                                                                                                                        \
            obj() *= new_obj.obj();                                                                                              \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator+=(name::value_type v)                                                                            \
        {                                                                                                                        \
            obj() += v;                                                                                                          \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator-=(name::value_type v)                                                                            \
        {                                                                                                                        \
            obj() -= v;                                                                                                          \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator/=(name::value_type v)                                                                            \
        {                                                                                                                        \
            obj() /= v;                                                                                                          \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper& operator*=(name::value_type v)                                                                            \
        {                                                                                                                        \
            obj() *= v;                                                                                                          \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper operator+(const name##Wrapper& new_obj) const                                                              \
        {                                                                                                                        \
            return obj() + new_obj.obj();                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper operator+(name::value_type v) const                                                                        \
        {                                                                                                                        \
            return obj() + v;                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper operator-(const name##Wrapper& new_obj) const                                                              \
        {                                                                                                                        \
            return obj() - new_obj.obj();                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper operator-(name::value_type v) const                                                                        \
        {                                                                                                                        \
            return obj() - v;                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper operator*(const name##Wrapper& new_obj) const                                                              \
        {                                                                                                                        \
            return obj() * new_obj.obj();                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper operator*(name::value_type v) const                                                                        \
        {                                                                                                                        \
            return obj() * v;                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper operator/(const name##Wrapper& new_obj) const                                                              \
        {                                                                                                                        \
            return obj() / new_obj.obj();                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper operator/(name::value_type v) const                                                                        \
        {                                                                                                                        \
            return obj() / v;                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper reverse_operator_sub(name::value_type f)                                                                   \
        {                                                                                                                        \
            return f - obj();                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        name##Wrapper reverse_operator_div(name::value_type f)                                                                   \
        {                                                                                                                        \
            return f / obj();                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        String as_string() const                                                                                                 \
        {                                                                                                                        \
            std::stringstream s;                                                                                                 \
            s << obj();                                                                                                          \
            return s.str();                                                                                                      \
        }                                                                                                                        \
                                                                                                                                 \
        name::value_type value_at(uint_t index) const                                                                            \
        {                                                                                                                        \
            return obj()[index];                                                                                                 \
        }                                                                                                                        \
                                                                                                                                 \
        name::value_type& value_at(uint_t index)                                                                                 \
        {                                                                                                                        \
            return obj()[index];                                                                                                 \
        }                                                                                                                        \
    };

    template<typename T>
    static void bind_glm_behaviours(ScriptClassRegistrar& registrar, const String& prop_type)
    {
        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<T>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, fmt::format("void f(const {}& in)", registrar.class_base_name()).c_str(),
                         ScriptClassRegistrar::constructor<T, const T&>, ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, fmt::format("void f({})", prop_type).c_str(),
                         ScriptClassRegistrar::constructor<T, typename T::value_type>, ScriptCallConv::CDECL_OBJFIRST);

        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<T>,
                         ScriptCallConv::CDECL_OBJFIRST);

        registrar.method("string as_string() const", &T::as_string, ScriptCallConv::THISCALL);
    }

    template<typename T>
    static void bind_glm_operators(ScriptClassRegistrar& registrar, const String& prop_type)
    {
        const String& name = registrar.class_base_name();

        registrar.opfunc(fmt::format("{}& opAssign(const {}& in)", name, name).c_str(), method_of<T&>(&T::operator=),
                         ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("bool opEquals(const {}& in) const", name).c_str(), method_of<bool>(&T::operator==),
                         ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("{}& opAddAssign(const {}& in)", name, name).c_str(),
                         method_of<T&, const T&>(&T::operator+=), ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{}& opAddAssign({})", name, prop_type).c_str(),
                         method_of<T&, typename T::value_type>(&T::operator+=), ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("{}& opSubAssign(const {}& in)", name, name).c_str(),
                         method_of<T&, const T&>(&T::operator-=), ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{}& opSubAssign({})", name, prop_type).c_str(),
                         method_of<T&, typename T::value_type>(&T::operator-=), ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("{}& opMulAssign(const {}& in)", name, name).c_str(),
                         method_of<T&, const T&>(&T::operator*=), ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{}& opMulAssign({})", name, prop_type).c_str(),
                         method_of<T&, typename T::value_type>(&T::operator*=), ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("{}& opDivAssign(const {}& in)", name, name).c_str(),
                         method_of<T&, const T&>(&T::operator/=), ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{}& opDivAssign({})", name, prop_type).c_str(),
                         method_of<T&, typename T::value_type>(&T::operator/=), ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("{} opAdd(const {}& in) const", name, name).c_str(), method_of<T, const T&>(&T::operator+),
                         ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opAdd({}) const", name, prop_type).c_str(),
                         method_of<T, typename T::value_type>(&T::operator+), ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opAdd_r({}) const", name, prop_type).c_str(),
                         method_of<T, typename T::value_type>(&T::operator+), ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("{} opSub(const {}& in) const", name, name).c_str(), method_of<T, const T&>(&T::operator-),
                         ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opSub({}) const", name, prop_type).c_str(),
                         method_of<T, typename T::value_type>(&T::operator-), ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opSub_r({}) const", name, prop_type).c_str(), &T::reverse_operator_sub,
                         ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("{} opMul(const {}& in) const", name, name).c_str(), method_of<T, const T&>(&T::operator*),
                         ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opMul({}) const", name, prop_type).c_str(),
                         method_of<T, typename T::value_type>(&T::operator*), ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opMul_r({}) const", name, prop_type).c_str(),
                         method_of<T, typename T::value_type>(&T::operator*), ScriptCallConv::THISCALL);

        registrar.opfunc(fmt::format("{} opDiv(const {}& in) const", name, name).c_str(), method_of<T, const T&>(&T::operator/),
                         ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opDiv({}) const", name, prop_type).c_str(),
                         method_of<T, typename T::value_type>(&T::operator/), ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opDiv_r({}) const", name, prop_type).c_str(), &T::reverse_operator_div,
                         ScriptCallConv::THISCALL);
    }

    template<typename T, typename ConstType, typename RefType>
    void bind_index_op(ScriptClassRegistrar& registrar, const char* const_type, const char* type)
    {
        registrar.opfunc(fmt::format("{} opIndex(uint index) const", const_type).c_str(), method_of<ConstType>(&T::value_at),
                         ScriptCallConv::THISCALL);
        registrar.opfunc(fmt::format("{} opIndex(uint index)", type).c_str(), method_of<RefType>(&T::value_at),
                         ScriptCallConv::THISCALL);
    }

    template<typename T>
    static void bind_vec1_props(ScriptClassRegistrar& registrar, const String& prop_type)
    {
        registrar.property(Strings::format("{} x", prop_type).c_str(), &T::x);
        registrar.property(Strings::format("{} r", prop_type).c_str(), &T::r);
        registrar.property(Strings::format("{} s", prop_type).c_str(), &T::s);
    }

    template<typename T>
    static void bind_vec2_props(ScriptClassRegistrar& registrar, const String& prop_type)
    {
        bind_vec1_props<T>(registrar, prop_type);
        registrar.property(Strings::format("{} y", prop_type).c_str(), &T::y);
        registrar.property(Strings::format("{} g", prop_type).c_str(), &T::g);
        registrar.property(Strings::format("{} t", prop_type).c_str(), &T::t);
    }

    template<typename T>
    static void bind_vec3_props(ScriptClassRegistrar& registrar, const String& prop_type)
    {
        bind_vec2_props<T>(registrar, prop_type);
        registrar.property(Strings::format("{} z", prop_type).c_str(), &T::z);
        registrar.property(Strings::format("{} b", prop_type).c_str(), &T::b);
        registrar.property(Strings::format("{} p", prop_type).c_str(), &T::p);
    }

    template<typename T>
    static void bind_vec4_props(ScriptClassRegistrar& registrar, const String& prop_type)
    {
        bind_vec3_props<T>(registrar, prop_type);
        registrar.property(Strings::format("{} w", prop_type).c_str(), &T::w);
        registrar.property(Strings::format("{} a", prop_type).c_str(), &T::a);
        registrar.property(Strings::format("{} q", prop_type).c_str(), &T::q);
    }
}// namespace Engine
