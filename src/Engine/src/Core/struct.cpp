#include <Core/exception.hpp>
#include <Core/group.hpp>
#include <Core/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>

namespace Engine
{
    static StructMap& internal_struct_map()
    {
        static StructMap map;
        return map;
    }

    static void on_destroy()
    {
        for (auto& [name, struct_entry] : internal_struct_map())
        {
            delete struct_entry;
        }

        internal_struct_map().clear();
    }

    static PostDestroyController destroy_struct_map(on_destroy);


    Struct::Struct(const Name& name, const Name& namespace_name, const Name& _parent)
        : _M_struct_constructor(nullptr), _M_namespace_name(namespace_name), _M_base_name(name), _M_parent(_parent)
    {
        _M_full_name = namespace_name.is_valid() ? Name(Strings::format("{}::{}", namespace_name.c_str(), name.c_str())) : name;
        _M_base_name_splitted = Strings::make_sentence(_M_base_name.to_string());

        if (_M_parent.is_valid())
        {
            parent();
        }

        internal_struct_map()[Strings::hash_of(_M_full_name)] = this;
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

    ENGINE_EXPORT Struct* Struct::static_find(const StringView& name, bool requred)
    {
        auto& map = internal_struct_map();
        auto it   = map.find(Strings::hash_of(name));
        if (it == map.end())
        {
            // Maybe initializer is not executed?
            InitializeController().require(Strings::format("{}{}", INITIALIZER_NAME_PREFIX, name.data()));
            it = map.find(Strings::hash_of(name));

            if (it != map.end())
                return it->second;

            if (requred)
            {
                throw EngineException(Strings::format("Failed to find struct '{}'", name.data()));
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

    void* Struct::create_struct() const
    {
        if (_M_struct_constructor)
        {
            return _M_struct_constructor();
        }

        return nullptr;
    }

    Struct& Struct::struct_constructor(void* (*constructor)())
    {
        _M_struct_constructor = constructor;
        return *this;
    }

    Struct& Struct::group(class Group* group)
    {
        if (_M_group)
        {
            _M_group->remove_struct(this);
        }

        _M_group = group;

        if (_M_group)
        {
            _M_group->add_struct(this);
        }
        return *this;
    }

    class Group* Struct::group() const
    {
        return _M_group;
    }

    size_t Struct::abstraction_level() const
    {
        size_t level = 1;

        Struct* next = parent();
        while (next)
        {
            ++level;
            next = next->parent();
        }

        return level;
    }

    Vector<Name> Struct::hierarchy(size_t offset) const
    {
        size_t level = abstraction_level();
        if (offset >= level)
            return {};

        level -= offset;

        Vector<Name> result;
        result.reserve(level);

        const Struct* current = this;
        while (current && level != 0)
        {
            result.emplace_back(current->_M_full_name);
            current = current->parent();
            --level;
        }

        return result;
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


    static FORCE_INLINE Property* find_prop_internal(Struct* self, const Name& name)
    {
        for (auto& prop : self->properties())
        {
            if (prop->name() == name)
            {
                return prop;
            }
        }

        return nullptr;
    }

    class Property* Struct::find_property(const Name& name, bool recursive)
    {
        if (recursive)
        {
            Struct* self   = this;
            Property* prop = nullptr;

            while (self && (prop = find_prop_internal(self, name)) == nullptr)
            {
                self = self->parent();
            }

            return prop;
        }
        else
        {
            return find_prop_internal(this, name);
        }
        return nullptr;
    }

    const Struct::GroupedPropertiesMap& Struct::grouped_properties() const
    {
        return _M_grouped_properties;
    }

    const StructMap& Struct::struct_map()
    {
        return internal_struct_map();
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
