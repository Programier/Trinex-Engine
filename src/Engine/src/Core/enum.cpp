#include <Core/constants.hpp>
#include <Core/enum.hpp>

namespace Engine
{
    static TreeMap<HashIndex, Enum>& enums_map()
    {
        static TreeMap<HashIndex, Enum> enums;
        return enums;
    }

    ENGINE_EXPORT Enum* Enum::create(const String& namespace_name, const String& name, const Vector<Enum::Entry>& entries)
    {
        Name full_name = Name(namespace_name.empty() ? name : namespace_name + "::" + name);

        Enum* _enum = find(full_name);

        if (!_enum)
        {
            _enum               = &(enums_map()[full_name.hash()]);
            _enum->_M_base_name = name;
            _enum->_M_full_name = full_name;

            if (!namespace_name.empty())
                _enum->_M_namespace_name = namespace_name;
            _enum->_M_entries = entries;

            Index index = 0;

            for(auto& entry : _enum->_M_entries)
            {
                entry.index = index;
                ++index;
            }
        }

        return _enum;
    }

    const Enum::Entry* Enum::entry(EnumerateType value) const
    {
        for (auto& ell : _M_entries)
        {
            if (ell.value == value)
                return &ell;
        }

        return nullptr;
    }

    const Enum::Entry* Enum::entry(const Name& name) const
    {
        for (auto& ell : _M_entries)
        {
            if (ell.name == name)
                return &ell;
        }

        return nullptr;
    }

    const Name& Enum::name() const
    {
        return _M_full_name;
    }

    const Name& Enum::namespace_name() const
    {
        return _M_namespace_name;
    }

    const Name& Enum::base_name() const
    {
        return _M_base_name;
    }

    const Vector<Enum::Entry>& Enum::entries() const
    {
        return _M_entries;
    }

    ENGINE_EXPORT Enum* Enum::find(const Name& name)
    {
        auto& map = enums_map();
        auto it   = map.find(name.hash());

        if (it == map.end())
            return nullptr;

        return &it->second;
    }
}// namespace Engine
