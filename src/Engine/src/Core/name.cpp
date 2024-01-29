#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/memory.hpp>
#include <Core/name.hpp>
#include <ScriptEngine/registrar.hpp>
#include <cstring>

namespace Engine
{
    static Vector<Name::Entry>& name_entries()
    {
        static Vector<Name::Entry> entries;
        return entries;
    }

    static const String& default_string()
    {
        static String default_name;
        return default_name;
    }

    static FORCE_INLINE void push_new_name(const char* name, size_t len, HashIndex hash)
    {
        name_entries().push_back(Name::Entry{String(name, len), hash});
    }


    ENGINE_EXPORT Name Name::none;

    Name Name::find_name(const String& name)
    {
        return find_name(name.c_str(), name.length());
    }

    Name Name::find_name(const char* name, size_t size)
    {
        Name out_name;
        HashIndex hash = memory_hash_fast(name, size == 0 ? std::strlen(name) : size, 0);

        Vector<Name::Entry>& name_table = name_entries();

        out_name._M_index = name_table.size();

        for (Index index = 0; index < out_name._M_index; ++index)
        {
            const Name::Entry& entry = name_table[index];

            if (entry.hash == hash)
            {
                out_name._M_index = index;
                return out_name;
            }
        }

        out_name._M_index = Constants::index_none;
        return out_name;
    }

    Name& Name::init(const char* name, size_t len)
    {
        if (len == 0 || name == nullptr)
        {
            _M_index = Constants::index_none;
            return *this;
        }

        HashIndex hash                  = memory_hash_fast(name, len, 0);
        Vector<Name::Entry>& name_table = name_entries();

        _M_index = name_table.size();

        for (Index index = 0; index < _M_index; ++index)
        {
            const Name::Entry& entry = name_table[index];

            if (entry.hash == hash)
            {
                _M_index = index;
                return *this;
            }
        }

        push_new_name(name, len, hash);
        return *this;
    }

    Name::Name() : _M_index(Constants::index_none)
    {}

    Name::Name(const String& name)
    {
        init(name.c_str(), name.length());
    }

    Name::Name(const StringView& name)
    {
        init(name.data(), name.length());
    }

    Name::Name(const char* name)
    {
        init(name, std::strlen(name));
    }

    Name::Name(const char* name, size_t size)
    {
        init(name, size);
    }

    Name& Name::operator=(const String& name)
    {
        return init(name.c_str(), name.length());
    }

    Name& Name::operator=(const StringView& name)
    {
        return init(name.data(), name.length());
    }

    Name& Name::operator=(const char* name)
    {
        return init(name, std::strlen(name));
    }

    Name::Name(const Name&) = default;
    Name::Name(Name&&)      = default;

    Name& Name::operator=(const Name&) = default;
    Name& Name::operator=(Name&&)      = default;

    bool Name::is_valid() const
    {
        return _M_index != Constants::index_none;
    }

    HashIndex Name::hash() const
    {
        return is_valid() ? name_entries()[_M_index].hash : Constants::invalid_hash;
    }

    bool Name::operator==(const String& name) const
    {
        if (is_valid())
        {
            return name == name_entries()[_M_index].name;
        }
        return false;
    }

    bool Name::operator==(const StringView& name) const
    {
        return equals(name);
    }

    bool Name::operator==(const char* name) const
    {
        if (is_valid())
        {
            return std::strcmp(name, name_entries()[_M_index].name.c_str()) == 0;
        }

        return false;
    }

    bool Name::operator!=(const String& name) const
    {
        if (is_valid())
        {
            return name != name_entries()[_M_index].name;
        }
        return false;
    }

    bool Name::operator!=(const StringView& name) const
    {
        return !equals(name);
    }

    bool Name::operator!=(const char* name) const
    {
        if (is_valid())
        {
            return std::strcmp(name, name_entries()[_M_index].name.c_str()) != 0;
        }

        return false;
    }

    bool Name::equals(const char* name, size_t len) const
    {
        if (is_valid())
        {
            const String& str = name_entries()[_M_index].name;
            return str.length() == len && std::memcmp(str.c_str(), name, len) == 0;
        }

        return false;
    }

    bool Name::equals(const char* name) const
    {
        return *this == name;
    }

    bool Name::equals(const String& name) const
    {
        return *this == name;
    }

    bool Name::equals(const StringView& name) const
    {
        if (is_valid())
        {
            const String& str = name_entries()[_M_index].name;
            return str == name;
        }

        return false;
    }

    bool Name::equals(const Name& name) const
    {
        return *this == name;
    }

    const Name& Name::to_string(String& out) const
    {
        if (is_valid())
        {
            out += name_entries()[_M_index].name;
        }

        return *this;
    }

    const String& Name::to_string() const
    {
        if (is_valid())
        {
            return name_entries()[_M_index].name;
        }

        return default_string();
    }

    const char* Name::c_str() const
    {
        return to_string().c_str();
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
        ar& is_valid;

        if (is_valid)
        {
            String string_name = name.to_string();
            ar& string_name;

            if (ar.is_reading())
            {
                name = string_name;
            }
        }

        return ar;
    }


    static void on_init()
    {
        ScriptClassRegistrar registrar("Engine::Name",
                                       ScriptClassRegistrar::create_type_info<Name>(
                                               ScriptClassRegistrar::Value | ScriptClassRegistrar::AppClassMoreConstructors));

        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Name>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, "void f(const string& in)",
                         ScriptClassRegistrar::constructor<Name, const String&>, ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, "void f(const Name& in)",
                         ScriptClassRegistrar::constructor<Name, const Name&>, ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Name>,
                         ScriptCallConv::CDECL_OBJFIRST);

        registrar.static_function("Name find_name(const string& in)", func_of<Name(const String&)>(Name::find_name));
        registrar.method("bool is_valid() const", &Name::is_valid);
        registrar.method("uint64 hash() const", &Name::hash);
        registrar.method("const string& to_string() const", method_of<const String&>(&Name::to_string));
        registrar.method("const Name& to_string(string& out) const", method_of<const Name&>(&Name::to_string));

        registrar.method("Engine::Name& opAssign(const Engine::Name& in)", method_of<Name&, Name, const Name&>(&Name::operator=));
        registrar.method("Engine::Name& opAssign(const string& in)", method_of<Name&, Name, const String&>(&Name::operator=));
        registrar.method("bool opEquals(const string& in) const", method_of<bool, Name, const String&>(&Name::operator==));
        registrar.method("bool opEquals(const Name& in) const", method_of<bool, Name, const Name&>(&Name::operator==));

        registrar.method("const string& opConv() const", &Name::operator const std::basic_string<char>&);
    }

    static ScriptEngineInitializeController controller(on_init, "Bind Engine::Name");
}// namespace Engine
