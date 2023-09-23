#include <Core/constants.hpp>
#include <Core/memory.hpp>
#include <Core/name.hpp>
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

    static FORCE_INLINE void push_new_name(const String& name, HashIndex hash)
    {
        name_entries().push_back(Name::Entry{name, hash});
    }

    Name& Name::init(const String& name)
    {
        HashIndex hash = memory_hash_fast(name.data(), name.length(), 0);

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

        push_new_name(name, hash);
        return *this;
    }

    Name::Name() : _M_index(Constants::index_none)
    {}

    Name::Name(const String& name)
    {
        init(name);
    }

    Name::Name(const char* name)
    {
        init(name);
    }

    Name& Name::operator=(const String& name)
    {
        return init(name);
    }

    Name& Name::operator=(const char* name)
    {
        return init(name);
    }

    Name::Name(const Name&) = default;
    Name::Name(Name&&)      = default;

    Name& Name::operator=(const Name&) = default;
    Name& Name::operator=(Name&&)      = default;

    bool Name::is_valid() const
    {
        return _M_index != Constants::index_none;
    }

    bool Name::operator==(const String& name) const
    {
        if (is_valid())
        {
            return name == name_entries()[_M_index].name;
        }
        return false;
    }

    bool Name::operator==(const char* name) const
    {
        if (is_valid())
        {
            return std::strcmp(name, name_entries()[_M_index].name.c_str()) != 0;
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

    bool Name::operator!=(const char* name) const
    {
        if (is_valid())
        {
            return std::strcmp(name, name_entries()[_M_index].name.c_str()) != 0;
        }

        return false;
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

    Name::operator const String&() const
    {
        return to_string();
    }

}// namespace Engine
