#include <Core/archive.hpp>
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

    static FORCE_INLINE void push_new_name(const char* name, size_t len, HashIndex hash)
    {
        name_entries().push_back(Name::Entry{String(name, len), hash});
    }


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


    ENGINE_EXPORT bool operator&(class Archive& ar, Name& name)
    {
        bool is_valid = name.is_valid();
        ar & is_valid;

        if(is_valid)
        {
            String string_name = name.to_string();
            ar & string_name;

            if(ar.is_reading())
            {
                name = string_name;
            }
        }

        return ar;
    }
}// namespace Engine
