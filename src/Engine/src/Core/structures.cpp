#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/property.hpp>
#include <Core/struct.hpp>
#include <Core/structures.hpp>

namespace Engine
{
    const BindLocation BindLocation::undefined = BindLocation(255, 255);

    BindLocation::BindLocation() : BindLocation(255, 255)
    {}

    BindLocation::BindLocation(BindingIndex in_binding, BindingIndex in_set) : binding(in_binding), set(in_set)
    {}

    bool BindLocation::operator==(const BindLocation& location) const
    {
        return location.id == id;
    }

    bool BindLocation::operator!=(const BindLocation& location) const
    {
        return location.id != id;
    }

    bool BindLocation::operator<(const BindLocation& location) const
    {
        return id < location.id;
    }

    bool BindLocation::operator<=(const BindLocation& location) const
    {
        return id <= location.id;
    }

    bool BindLocation::operator>(const BindLocation& location) const
    {
        return id > location.id;
    }

    bool BindLocation::operator>=(const BindLocation& location) const
    {
        return id >= location.id;
    }

    bool BindLocation::is_valid() const
    {
        return (*this) != undefined;
    }

    ENGINE_EXPORT bool operator&(class Archive& ar, ShaderDefinition& definition)
    {
        ar & definition.key;
        ar & definition.value;
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, MaterialScalarParametersInfo& info)
    {
        ar & info.m_binding_index;
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, MaterialParameterInfo& info)
    {
        ar & info.type;
        ar & info.name;
        ar & info.size;
        ar & info.offset;
        ar & info.location;
        return ar;
    }

    MaterialParameterInfo::MaterialParameterInfo()
        : type(MaterialParameterType::Undefined), name(""), size(0), offset(Constants::offset_none),
          location(BindLocation(255, 255))
    {}

    implement_struct(ShaderDefinition, Engine, ).push([]() {
        Struct* self = Struct::static_find("Engine::ShaderDefinition", true);
        self->add_property(new StringProperty("Key", "Key of definition", &ShaderDefinition::key));
        self->add_property(new StringProperty("Value", "Value of definition", &ShaderDefinition::value));
    });
}// namespace Engine
