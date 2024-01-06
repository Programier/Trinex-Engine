#include <Core/property.hpp>

namespace Engine
{

    Property::Property(const Name& name, const String& description, size_t offset, BitMask flags)
        : _M_name(name), _M_description(description), _M_offset(offset), _M_flags(flags)
    {}

    const Name& Property::name() const
    {
        return _M_name;
    }

    const String& Property::description() const
    {
        return _M_description;
    }

    size_t Property::offset() const
    {
        return _M_offset;
    }

    const Flags& Property::flags() const
    {
        return _M_flags;
    }

    Property::~Property()
    {}

#define declare_property_type(native_type, enum_type)                                                                            \
    size_t enum_type##Property::element_size() const                                                                             \
    {                                                                                                                            \
        return sizeof(native_type);                                                                                              \
    }                                                                                                                            \
    Property::Type enum_type##Property::type() const                                                                             \
    {                                                                                                                            \
        return Property::Type::enum_type;                                                                                        \
    }                                                                                                                            \
    size_t enum_type##Property::min_alignment() const                                                                            \
    {                                                                                                                            \
        return alignof(native_type);                                                                                             \
    }                                                                                                                            \
    PropertyValue enum_type##Property::property_value(const Object* object) const                                                \
    {                                                                                                                            \
        if (object)                                                                                                              \
        {                                                                                                                        \
            return *reinterpret_cast<const native_type*>(reinterpret_cast<const byte*>(object) + offset());                      \
        }                                                                                                                        \
        return {};                                                                                                               \
    }                                                                                                                            \
    bool enum_type##Property::property_value(Object* object, const PropertyValue& property_value)                                \
    {                                                                                                                            \
        bool success = false;                                                                                                    \
        if (object && !flags().has_all(Flag::IsConst))                                                                           \
        {                                                                                                                        \
            native_type* prop_address = reinterpret_cast<native_type*>(reinterpret_cast<byte*>(object) + offset());              \
            (*prop_address)           = std::any_cast<native_type>(property_value);                                              \
            success                   = true;                                                                                    \
        }                                                                                                                        \
        return success;                                                                                                          \
    }

    declare_property_type(byte, Byte);
    declare_property_type(int_t, Int);
    declare_property_type(bool, Bool);
    declare_property_type(float, Float);
    declare_property_type(Vector2D, Vec2);
    declare_property_type(Vector3D, Vec3);
    declare_property_type(Vector4D, Vec4);
    declare_property_type(String, String);
    declare_property_type(Path, Path);
}// namespace Engine
