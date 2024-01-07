#include <Core/exception.hpp>
#include <Core/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>

namespace Engine
{
    using StructMap = Map<String, Struct*>;

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


    Struct::Struct(const Name& name, const Name& namespace_name, const Name& _parent)
        : _M_namespace_name(namespace_name), _M_base_name(name), _M_parent(_parent)
    {
        _M_full_name = namespace_name.is_valid() ? Name(Strings::format("{}::{}", namespace_name.c_str(), name.c_str())) : name;
        _M_base_name_splitted = Strings::make_sentence(_M_base_name.to_string());

        if (_M_parent.is_valid())
        {
            parent();
        }

        struct_map()[_M_full_name] = this;
    }

    Struct::Struct(const Name& name, const Name& namespace_name, Struct* parent) : Struct(name, namespace_name)
    {
        _M_parent_struct = parent;
        if (_M_parent_struct)
        {
            _M_parent = _M_parent_struct->name();
        }
    }

    ENGINE_EXPORT Struct* Struct::create(const Name& name, const Name& namespace_name, const Name& parent)
    {
        Name full_name = namespace_name.is_valid() ? Name(Strings::format("{}::{}", namespace_name.c_str(), name.c_str())) : name;
        Struct* self   = static_find(full_name);

        if (!self)
        {
            self = new Struct(name, namespace_name, parent);
        }

        return self;
    }

    ENGINE_EXPORT Struct* Struct::static_find(const String& name, bool requred)
    {
        auto& map = struct_map();
        auto it   = map.find(name);
        if (it == map.end())
        {
            // Maybe initializer is not executed?
            InitializeController().require(Strings::format("{}{}", INITIALIZER_NAME_PREFIX, name.c_str()));
            it = map.find(name);

            if (it != map.end())
                return it->second;

            if (requred)
            {
                throw EngineException(Strings::format("Failed to find struct '{}'", name.c_str()));
            }
            return nullptr;
        }
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
            _M_parent_struct = static_find(_M_parent);
        }
        return _M_parent_struct;
    }

    bool Struct::is_a(const Struct* other) const
    {
        const Struct* current = this;
        while (current && current != other)
        {
            current = current->parent();
        }
        return current != nullptr;
    }

    bool Struct::is_class() const
    {
        return false;
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
