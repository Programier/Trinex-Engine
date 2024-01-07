#include <Core/constants.hpp>
#include <Core/enum.hpp>

namespace Engine
{
    using EnumMap = Map<Name, Enum*, Name::HashFunction>;
    static EnumMap& enums_map()
    {
        static EnumMap enums;
        return enums;
    }

    static void on_destroy()
    {
        for (auto& [name, enum_entry] : enums_map())
        {
            delete enum_entry;
        }

        enums_map().clear();
    }

    static PostDestroyController destroy_struct_map(on_destroy);

    ENGINE_EXPORT Enum* Enum::create(const String& namespace_name, const String& name, const Vector<Enum::Entry>& entries)
    {
        Name full_name = Name(namespace_name.empty() ? name : namespace_name + "::" + name);
        Enum* _enum    = find(full_name);

        if (!_enum)
        {
            _enum                  = new Enum();
            enums_map()[full_name] = _enum;

            _enum->_M_base_name = name;
            _enum->_M_full_name = full_name;

            if (!namespace_name.empty())
                _enum->_M_namespace_name = namespace_name;
            _enum->_M_entries = entries;

            Index index = 0;

            for (auto& entry : _enum->_M_entries)
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
        auto it   = map.find(name);

        if (it == map.end())
            return nullptr;

        return it->second;
    }
}// namespace Engine
