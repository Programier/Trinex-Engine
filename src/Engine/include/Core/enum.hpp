#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/name.hpp>

namespace Engine
{
    class ENGINE_EXPORT Enum final
    {
    public:
        struct Entry {
            Name name;
            EnumerateType value;

            FORCE_INLINE Entry() : name(Name::none), value(0)
            {}

            template<typename EnumValue = EnumerateType>
            FORCE_INLINE Entry(const Name& name, EnumValue value) : name(name), value(static_cast<EnumerateType>(value))
            {}

            template<typename EnumValue = EnumerateType>
            FORCE_INLINE Entry(EnumValue value, const Name& name) : name(name), value(static_cast<EnumerateType>(value))
            {}
        };

    private:
        Name m_full_name;
        Name m_namespace_name;
        Name m_base_name;

        TreeMap<Name, Index> m_entries_by_name;
        TreeMap<EnumerateType, Index> m_entries_by_value;
        Vector<Entry> m_entries;

    public:
        static ENGINE_EXPORT Enum* create(const String& namespace_name, const String& name, const Vector<Enum::Entry>& entries);
        Index index_of(const Name& name) const;
        Index index_of(EnumerateType value) const;
        const Entry* entry(EnumerateType value) const;
        const Entry* entry(const Name& name) const;
        const Entry* create_entry(const Name& name, EnumerateType value);

        const Name& name() const;
        const Name& namespace_name() const;
        const Name& base_name() const;
        const Vector<Enum::Entry>& entries() const;
        static ENGINE_EXPORT Enum* static_find(const String& name, bool required = false);
    };

#define implement_enum(enum_name, namespace_name, ...)                                                                           \
    static Engine::ClassInitializeController initialize_##enum_name = Engine::ClassInitializeController(                         \
            []() { Enum::create(#namespace_name, #enum_name, Vector<Engine::Enum::Entry>({__VA_ARGS__})); },                     \
            ENTITY_INITIALIZER_NAME(enum_name, namespace_name))
}// namespace Engine
