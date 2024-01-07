#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/name.hpp>

namespace Engine
{
    class ENGINE_EXPORT Struct final
    {
    public:
        using GroupedPropertiesMap = TreeMap<Name, Vector<class Property*>, Name::Less>;

    protected:
        String _M_base_name_splitted;

        Name _M_full_name;
        Name _M_namespace_name;
        Name _M_base_name;
        Name _M_parent;

        mutable Struct* _M_parent_struct = nullptr;

        Vector<class Property*> _M_properties;
        GroupedPropertiesMap _M_grouped_properties;

        Struct(const Name& name, const String& namespace_name, const Name& parent = Name::none);

    public:
        static ENGINE_EXPORT Struct* create(const Name& name, const String& namespace_name, const Name& parent = Name::none);
        static ENGINE_EXPORT Struct* find(const Name& name);

        const String& base_name_splitted() const;
        const Name& name() const;
        const Name& namespace_name() const;
        const Name& base_name() const;
        const Name& parent_name() const;
        Struct* parent() const;

        Struct& add_property(Property* prop);
        const Vector<class Property*>& properties() const;
        const GroupedPropertiesMap& grouped_properties() const;

        template<typename... Args>
        Struct& add_properties(Args&&... args)
        {
            (add_property(std::forward<Args>(args)), ...);
            return *this;
        }

        ~Struct();
    };
}// namespace Engine
