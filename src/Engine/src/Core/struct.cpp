#include <Core/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>

namespace Engine
{
    using StructMap = Map<Name, Struct*, Name::HashFunction>;

    static StructMap& struct_map()
    {
        static StructMap map;
        return map;
    }

    static void on_destroy()
    {
        for (auto& [name, struct_entry] : struct_map())
        {
            delete struct_entry;
        }

        struct_map().clear();
    }

    static PostDestroyController destroy_struct_map(on_destroy);


    Struct::Struct(const Name& name, const String& namespace_name, const Name& parent)
    {
        _M_full_name = namespace_name.empty() ? name : Name(Strings::format("{}::{}", namespace_name.c_str(), name.c_str()));
        _M_base_name = name;
    }

    ENGINE_EXPORT Struct* Struct::create(const Name& name, const String& namespace_name, const Name& parent)
    {
        Name full_name = namespace_name.empty() ? name : Name(Strings::format("{}::{}", namespace_name.c_str(), name.c_str()));
        Struct* self   = find(full_name);

        if (!self)
        {
            self                    = new Struct(name, namespace_name, parent);
            self->_M_base_name      = name;
            self->_M_namespace_name = namespace_name;
            self->_M_full_name      = full_name;
            self->_M_parent         = parent;
            self->_M_base_name_splitted = Strings::make_sentence(self->_M_base_name.to_string());

            struct_map()[full_name] = self;
        }

        return self;
    }

    ENGINE_EXPORT Struct* Struct::find(const Name& name)
    {
        auto& map = struct_map();
        auto it   = map.find(name);
        if (it == map.end())
            return nullptr;
        return it->second;
    }

    const String& Struct::base_name_splitted() const
    {
        return _M_base_name_splitted;
    }

    const Name& Struct::name() const
    {
        return _M_full_name;
    }

    const Name& Struct::namespace_name() const
    {
        return _M_namespace_name;
    }

    const Name& Struct::base_name() const
    {
        return _M_base_name;
    }

    const Name& Struct::parent_name() const
    {
        return _M_parent;
    }

    Struct* Struct::parent() const
    {
        if (_M_parent_struct == nullptr)
        {
            _M_parent_struct = find(_M_parent);
        }
        return _M_parent_struct;
    }

    Struct& Struct::add_property(Property* prop)
    {
        _M_properties.push_back(prop);
        _M_grouped_properties[prop->group()].push_back(prop);
        return *this;
    }

    const Vector<class Property*>& Struct::properties() const
    {
        return _M_properties;
    }

    const Struct::GroupedPropertiesMap& Struct::grouped_properties() const
    {
        return _M_grouped_properties;
    }

    Struct::~Struct()
    {
        for (auto* prop : _M_properties)
        {
            delete prop;
        }

        _M_properties.clear();
    }
}// namespace Engine
