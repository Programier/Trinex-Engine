#pragma once
#include <Core/engine_types.hpp>
#include <Core/flags.hpp>
#include <Core/name.hpp>

namespace Engine
{
    using PropertyValue = Any;

    class ENGINE_EXPORT Property
    {
    public:
        enum class Type
        {
            Byte,
            Int,
            Bool,
            Float,
            Vec2,
            Vec3,
            Vec4,
            String,
            Path,
        };

        enum Flag
        {
            IsConst   = (1 << 0),
            IsPrivate = (1 << 1),
        };

    private:
        Name _M_name;
        String _M_description;
        size_t _M_offset;
        Flags _M_flags;

    public:
        Property(const Name& name, const String& description, size_t offset, BitMask flags = 0);

        const Name& name() const;
        const String& description() const;
        size_t offset() const;
        const Flags& flags() const;

        virtual PropertyValue property_value(const Object* object) const                 = 0;
        virtual bool property_value(Object* object, const PropertyValue& property_value) = 0;
        virtual size_t element_size() const                                              = 0;
        virtual size_t min_alignment() const                                             = 0;
        virtual Property::Type type() const                                              = 0;
        virtual ~Property();
    };

#define declare_property_type(enum_type)                                                                                         \
    class ENGINE_EXPORT enum_type##Property : public Property                                                                    \
    {                                                                                                                            \
    public:                                                                                                                      \
        using Property::Property;                                                                                                \
        PropertyValue property_value(const Object* object) const override;                                                       \
        bool property_value(Object* object, const PropertyValue& property_value) override;                                       \
        size_t element_size() const override;                                                                                    \
        size_t min_alignment() const override;                                                                                   \
        Property::Type type() const override;                                                                                    \
    }

    declare_property_type(Byte);
    declare_property_type(Int);
    declare_property_type(Bool);
    declare_property_type(Float);
    declare_property_type(Vec2);
    declare_property_type(Vec3);
    declare_property_type(Vec4);
    declare_property_type(String);
    declare_property_type(Path);

#undef declare_property_type
}// namespace Engine
