#include <Core/constants.hpp>
#include <Core/enum.hpp>
#include <Core/exception.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
    using EnumMap = Map<String, Enum*>;
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

            _enum->m_base_name = name;
            _enum->m_full_name = full_name;

            if (!namespace_name.empty())
                _enum->m_namespace_name = namespace_name;
            _enum->m_entries = entries;

            Index index = 0;

            for (auto& entry : _enum->m_entries)
            {
                entry.index = index;
                ++index;
            }
        }

        return _enum;
    }

    const Enum::Entry* Enum::entry(EnumerateType value) const
    {
        for (auto& ell : m_entries)
        {
            if (ell.value == value)
                return &ell;
        }

        return nullptr;
    }

    const Enum::Entry* Enum::entry(const Name& name) const
    {
        for (auto& ell : m_entries)
        {
            if (ell.name == name)
                return &ell;
        }

        return nullptr;
    }

    const Name& Enum::name() const
    {
        return m_full_name;
    }

    const Name& Enum::namespace_name() const
    {
        return m_namespace_name;
    }

    const Name& Enum::base_name() const
    {
        return m_base_name;
    }

    const Vector<Enum::Entry>& Enum::entries() const
    {
        return m_entries;
    }

    ENGINE_EXPORT Enum* Enum::find(const String& name, bool required)
    {
        auto& map = enums_map();
        auto it   = map.find(name);

        if (it == map.end())
        {
            // Maybe initializer is not executed?
            InitializeController().require(Strings::format("{}{}", INITIALIZER_NAME_PREFIX, name.c_str()));
            it = map.find(name);

            if (it != map.end())
                return it->second;

            if (required)
            {
                throw EngineException(Strings::format("Failed to find enum '{}'", name.c_str()));
            }
            return nullptr;
        }

        return it->second;
    }
}// namespace Engine
