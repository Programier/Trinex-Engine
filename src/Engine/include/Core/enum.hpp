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
            Index index;

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
        Name _M_full_name;
        Name _M_namespace_name;
        Name _M_base_name;

        Vector<Entry> _M_entries;

    public:
        static ENGINE_EXPORT Enum* create(const String& namespace_name, const String& name, const Vector<Enum::Entry>& entries);
        const Entry* entry(EnumerateType value) const;
        const Entry* entry(const Name& name) const;

        const Name& name() const;
        const Name& namespace_name() const;
        const Name& base_name() const;
        const Vector<Enum::Entry>& entries() const;
        static ENGINE_EXPORT Enum* find(const String& name, bool required = false);
    };

#define implement_enum(enum_name, namespace_name, ...)                                                                           \
    static Engine::InitializeController initialize_##enum_name = Engine::InitializeController(                                   \
            []() { Enum::create(#namespace_name, #enum_name, Vector<Engine::Enum::Entry>({__VA_ARGS__})); },                     \
            ENTITY_INITIALIZER_NAME(enum_name, namespace_name))
}// namespace Engine
