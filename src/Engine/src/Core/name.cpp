#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/memory.hpp>
#include <Core/name.hpp>
#include <ScriptEngine/registrar.hpp>
#include <cstring>

namespace Engine
{
#define declare_name(name) const Name Name::name = #name
    declare_name(undefined);
    declare_name(out_of_range);
    declare_name(model);
    declare_name(texture);
    declare_name(color);
    declare_name(ambient_color);
    declare_name(radius);
    declare_name(height);
    declare_name(cutoff);
    declare_name(inverse_rotation);
    declare_name(scale);
    declare_name(intensivity);
    declare_name(location);
    declare_name(direction);

    static Vector<Name::Entry>& name_entries()
    {
        static Vector<Name::Entry> entries;
        return entries;
    }

    static MultiMap<HashIndex, Index>& name_index_map()
    {
        static MultiMap<HashIndex, Index> indices;
        return indices;
    }

    static const String& default_string()
    {
        static String default_name;
        return default_name;
    }

    static FORCE_INLINE void push_new_name(const char* name, size_t len, HashIndex hash)
    {
        name_index_map().insert({hash, name_entries().size()});
        name_entries().push_back(Name::Entry{String(name, len), hash});
    }


    ENGINE_EXPORT Name Name::none;

    Name Name::find_name(const StringView& name)
    {
        Name out_name;
        HashIndex hash = memory_hash_fast(name.data(), name.length(), 0);

        Vector<Name::Entry>& name_table = name_entries();

        out_name.m_index = name_table.size();

        for (Index index = 0; index < out_name.m_index; ++index)
        {
            const Name::Entry& entry = name_table[index];

            if (entry.hash == hash)
            {
                out_name.m_index = index;
                return out_name;
            }
        }

        out_name.m_index = Constants::index_none;
        return out_name;
    }


    Name& Name::init(const StringView& view)
    {
        if (view.empty())
        {
            m_index = Constants::index_none;
            return *this;
        }

        HashIndex hash                      = memory_hash_fast(view.data(), view.length(), 0);
        Vector<Name::Entry>& name_table     = name_entries();
        MultiMap<HashIndex, Index>& indices = name_index_map();

        m_index = name_table.size();

        auto range = indices.equal_range(hash);

        while (range.first != range.second)
        {
            Index index = range.first->second;

            if (name_table[index].name == view)
            {
                m_index = index;
                return *this;
            }

            ++range.first;
        }

        push_new_name(view.data(), view.length(), hash);
        return *this;
    }

    Name::Name() : m_index(Constants::index_none)
    {}

    Name::Name(const char* name) : Name(StringView(name))
    {}

    Name::Name(const char* name, size_t len) : Name(StringView(name, len))
    {}

    Name::Name(const String& name) : Name(StringView(name))
    {}

    Name::Name(const StringView& name)
    {
        init(name);
    }

    Name& Name::operator=(const char* name)
    {
        return init(name);
    }

    Name& Name::operator=(const String& name)
    {
        return init(name);
    }

    Name& Name::operator=(const StringView& name)
    {
        return init(name);
    }

    Name::Name(const Name&) = default;
    Name::Name(Name&&)      = default;

    Name& Name::operator=(const Name&) = default;
    Name& Name::operator=(Name&&)      = default;

    bool Name::is_valid() const
    {
        return m_index != Constants::index_none;
    }

    HashIndex Name::hash() const
    {
        return is_valid() ? name_entries()[m_index].hash : Constants::invalid_hash;
    }

    bool Name::operator==(const StringView& name) const
    {
        return equals(name);
    }

    bool Name::operator!=(const StringView& name) const
    {
        return !equals(name);
    }

    bool Name::operator==(const char* name) const
    {
        return equals(name);
    }

    bool Name::operator!=(const char* name) const
    {
        return !equals(name);
    }

    bool Name::operator==(const String& name) const
    {
        return equals(name);
    }

    bool Name::operator!=(const String& name) const
    {
        return !equals(name);
    }

    bool Name::equals(const String& name) const
    {
        return equals(StringView(name));
    }

    bool Name::equals(const char* name) const
    {
        return equals(StringView(name));
    }

    bool Name::equals(const char* name, size_t len) const
    {
        return equals(StringView(name, len));
    }

    bool Name::equals(const StringView& name) const
    {
        if (is_valid())
        {
            const String& str = name_entries()[m_index].name;
            return str == name;
        }

        return false;
    }

    bool Name::equals(const Name& name) const
    {
        name.equals("Hello");
        return *this == name;
    }

    const Name& Name::to_string(String& out) const
    {
        if (is_valid())
        {
            out += name_entries()[m_index].name;
        }

        return *this;
    }

    const String& Name::to_string() const
    {
        if (is_valid())
        {
            return name_entries()[m_index].name;
        }

        return default_string();
    }

    const char* Name::c_str() const
    {
        return to_string().c_str();
    }

    Name::operator StringView() const
    {
        return StringView(to_string());
    }

    Name::operator const String&() const
    {
        return to_string();
    }

    const Vector<Name::Entry>& Name::entries()
    {
        return name_entries();
    }

    ENGINE_EXPORT bool operator&(class Archive& ar, Name& name)
    {
        bool is_valid = name.is_valid();
        ar & is_valid;

        if (is_valid)
        {
            String string_name = name.to_string();
            ar & string_name;

            if (ar.is_reading())
            {
                name = string_name;
            }
        }

        return ar;
    }


    static void on_init()
    {

        ScriptEngineInitializeController().require("Bind Engine::StringView");

        ScriptClassRegistrar registrar(
                "Engine::Name", ScriptClassRegistrar::create_type_info<Name>(ScriptClassRegistrar::Value |
                                                                             ScriptClassRegistrar::AppClassMoreConstructors |
                                                                             ScriptClassRegistrar::AppClassAllInts));

        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Name>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, "void f(const string& in)",
                         ScriptClassRegistrar::constructor<Name, const String&>, ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, "void f(const StringView& in)",
                         ScriptClassRegistrar::constructor<Name, const StringView&>, ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, "void f(const Name& in)",
                         ScriptClassRegistrar::constructor<Name, const Name&>, ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Name>,
                         ScriptCallConv::CDECL_OBJFIRST);

        registrar.static_function("Name find_name(const StringView& in)", func_of<Name(const StringView&)>(Name::find_name));
        registrar.method("bool is_valid() const", &Name::is_valid);
        registrar.method("uint64 hash() const", &Name::hash);
        registrar.method("const string& to_string() const", method_of<const String&>(&Name::to_string));
        registrar.method("const Name& to_string(string& out) const", method_of<const Name&>(&Name::to_string));

        registrar.method("Engine::Name& opAssign(const Engine::Name& in)", method_of<Name&, Name, const Name&>(&Name::operator=));
        registrar.method("Engine::Name& opAssign(const StringView& in)",
                         method_of<Name&, Name, const StringView&>(&Name::operator=));
        registrar.method("Engine::Name& opAssign(const string& in)", method_of<Name&, Name, const String&>(&Name::operator=));
        registrar.method("bool opEquals(const StringView& in) const",
                         method_of<bool, Name, const StringView&>(&Name::operator==));
        registrar.method("bool opEquals(const string& in) const", method_of<bool, Name, const String&>(&Name::operator==));
        registrar.method("bool opEquals(const Name& in) const", method_of<bool, Name, const Name&>(&Name::operator==));

        registrar.method("const string& opConv() const", &Name::operator const std::basic_string<char>&);
    }

    static ScriptEngineInitializeController controller(on_init, "Bind Engine::Name");
}// namespace Engine
