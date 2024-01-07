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
            Enum,
            Object,
            ObjectReference,
        };

        enum Flag
        {
            IsPrivate = (1 << 0),
            IsConst   = (1 << 1),
        };

    private:
        Name _M_name;
        Name _M_group;
        String _M_description;
        Flags _M_flags;

    protected:
        static bool is_valid_object(const class Class* instance, const class Object* object);

    public:
        Property(const Name& name, const String& description, const Name& group = Name::none, BitMask flags = 0);

        const Name& name() const;
        const Name& group() const;
        const String& description() const;
        const Flags& flags() const;

        virtual PropertyValue property_value(const Object* object) const                 = 0;
        virtual bool property_value(Object* object, const PropertyValue& property_value) = 0;
        virtual size_t element_size() const                                              = 0;
        virtual size_t min_alignment() const                                             = 0;
        virtual Property::Type type() const                                              = 0;
        virtual bool is_const() const                                                    = 0;

        virtual void* property_class() const;
        virtual ~Property();
    };

    template<typename Instance, typename PropType, typename OutputType = PropType>
    class TypedProperty : public Property
    {
    private:
        PropType Instance::*_M_address = nullptr;

    public:
        FORCE_INLINE TypedProperty(const Name& name, const String& description, PropType Instance::*prop,
                                   const Name& group = Name::none, BitMask flags = 0)
            : Property(name, description, group, flags)
        {
            _M_address = prop;
        }

        FORCE_INLINE size_t element_size() const override
        {
            return sizeof(PropType);
        }

        FORCE_INLINE size_t min_alignment() const override
        {
            return alignof(PropType);
        }

        FORCE_INLINE bool is_const() const override
        {
            return flags()(IsConst) || std::is_const_v<PropType>;
        }

        PropertyValue property_value(const Object* object) const override
        {
            if (!is_valid_object(Instance::static_class_instance(), object))
                return {};

            const Instance* instance = reinterpret_cast<const Instance*>(object);
            return static_cast<OutputType>(instance->*_M_address);
        }

        bool property_value(Object* object, const PropertyValue& property_value) override
        {
            if constexpr (std::is_const_v<PropType>)
            {
                return false;
            }
            else
            {
                if (!is_valid_object(Instance::static_class_instance(), object))
                    return false;
                Instance* instance    = reinterpret_cast<Instance*>(object);
                instance->*_M_address = (PropType)(std::any_cast<const OutputType&>(property_value));
                return true;
            }
        }
    };


#define declare_property_type(native_type, enum_type)                                                                            \
    template<typename Instance, typename PropType = native_type>                                                                 \
    class enum_type##Property : public TypedProperty<Instance, PropType>                                                         \
    {                                                                                                                            \
    public:                                                                                                                      \
        static_assert(std::is_same_v<PropType, native_type>, "Property type must be " #native_type);                             \
        enum_type##Property(const Name& name, const String& description, PropType(Instance::*prop),                              \
                            const Name& group = Name::none, BitMask flags = 0)                                                   \
            : TypedProperty<Instance, PropType>(name, description, prop, group, flags)                                           \
        {}                                                                                                                       \
        FORCE_INLINE Property::Type type() const override                                                                        \
        {                                                                                                                        \
            return Property::Type::enum_type;                                                                                    \
        }                                                                                                                        \
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

#undef declare_property_type


    template<typename Instance, typename PropType>
    class EnumProperty : public TypedProperty<Instance, PropType, EnumerateType>
    {
    private:
        class Enum* _M_enum;

    public:
        static_assert(std::is_enum_v<PropType>, "PropType must be enum type");

        EnumProperty(const Name& name, const String& description, PropType(Instance::*prop), class Enum* _enum,
                     const Name& group = Name::none, BitMask flags = 0)

            : TypedProperty<Instance, PropType, EnumerateType>(name, description, prop, group, flags)
        {
            _M_enum = _enum;
        }

        FORCE_INLINE Property::Type type() const override
        {
            return Property::Type::Enum;
        }

        void* property_class() const override
        {
            return _M_enum;
        }
    };


    template<typename Instance, typename ObjectType>
    class ObjectReferenceProperty : public TypedProperty<Instance, ObjectType*, Object*>
    {
    public:
        static_assert(std::is_base_of_v<class Object, ObjectType>, "PropType must be object pointer");

        ObjectReferenceProperty(const Name& name, const String& description, ObjectType*(Instance::*prop),
                                const Name& group = Name::none, BitMask flags = 0)
            : TypedProperty<Instance, ObjectType*, Object*>(name, description, prop, group, flags)
        {}

        FORCE_INLINE Property::Type type() const override
        {
            return Property::Type::ObjectReference;
        }
    };

    template<typename Instance, typename ObjectType>
    class ObjectProperty : public ObjectReferenceProperty<Instance, ObjectType>
    {
    public:
        static_assert(std::is_base_of_v<class Object, ObjectType>, "PropType must be object pointer");

        ObjectProperty(const Name& name, const String& description, ObjectType*(Instance::*prop), const Name& group = Name::none,
                       BitMask flags = 0)
            : ObjectReferenceProperty<Instance, ObjectType>(name, description, prop, group, flags)
        {}

        FORCE_INLINE bool property_value(Object* object, const PropertyValue& property_value) override
        {
            return false;
        }

        FORCE_INLINE Property::Type type() const override
        {
            return Property::Type::Object;
        }
    };
}// namespace Engine
