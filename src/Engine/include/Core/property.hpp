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
            Byte            = 1,
            SignedByte      = 2,
            Int8            = SignedByte,
            UnsignedInt8    = Byte,
            Int16           = 3,
            UnsignedInt16   = 4,
            Int             = 5,
            UnsignedInt     = 6,
            Int32           = 7,
            UnsignedInt32   = 8,
            Int64           = 9,
            UnsignedInt64   = 10,
            Bool            = 11,
            Float           = 12,
            Vec2            = 13,
            Vec3            = 14,
            Vec4            = 15,
            String          = 16,
            Path            = 17,
            Enum            = 18,
            Object          = 19,
            ObjectReference = 20,
            Struct          = 21,
            Array           = 22,
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

        virtual PropertyValue property_value(const void* object) const                 = 0;
        virtual bool property_value(void* object, const PropertyValue& property_value) = 0;
        virtual size_t element_size() const                                            = 0;
        virtual size_t min_alignment() const                                           = 0;
        virtual Property::Type type() const                                            = 0;
        virtual bool is_const() const                                                  = 0;

        virtual void* property_class() const;
        virtual ~Property();
    };

    template<typename PropType>
    class BaseTypedProperty : public Property
    {
    public:
        using Property::Property;

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
    };

    // If _M_address is nullptr, then _object is property address
    template<typename Instance, typename PropType, typename OutputType = PropType>
    class TypedProperty : public BaseTypedProperty<PropType>
    {
    private:
        PropType Instance::*_M_address = nullptr;

    public:
        FORCE_INLINE TypedProperty(const Name& name, const String& description, PropType Instance::*prop,
                                   const Name& group = Name::none, BitMask flags = 0)
            : BaseTypedProperty<PropType>(name, description, group, flags)
        {
            _M_address = prop;
        }

        PropertyValue property_value(const void* _object) const override
        {
            if (_object == nullptr)
            {
                return {};
            }

            if (_M_address)
            {
                if constexpr (std::is_base_of_v<Object, Instance>)
                {
                    const Object* object = reinterpret_cast<const Object*>(_object);
                    if (!this->is_valid_object(Instance::static_class_instance(), object))
                        return {};
                }

                const Instance* instance = reinterpret_cast<const Instance*>(_object);
                return static_cast<OutputType>(instance->*_M_address);
            }
            else
            {
                const PropType* address = reinterpret_cast<const PropType*>(_object);
                return static_cast<OutputType>(*address);
            }
        }

        bool property_value(void* _object, const PropertyValue& property_value) override
        {
            if constexpr (std::is_const_v<PropType>)
            {
                return false;
            }
            else
            {
                if (this->flags()(Property::IsConst) || !property_value.has_value() || !_object)
                    return false;

                if (_M_address)
                {
                    if constexpr (std::is_base_of_v<Object, Instance>)
                    {
                        Object* object = reinterpret_cast<Object*>(_object);
                        if (!this->is_valid_object(Instance::static_class_instance(), object))
                            return false;
                    }
                    Instance* instance    = reinterpret_cast<Instance*>(_object);
                    instance->*_M_address = (PropType) (std::any_cast<const OutputType&>(property_value));
                }
                else
                {
                    (*reinterpret_cast<PropType*>(_object)) = (PropType) (std::any_cast<const OutputType&>(property_value));
                }
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
    };                                                                                                                           \
    template<typename PropType = native_type>                                                                                    \
    class enum_type##ArrayElementProperty : public enum_type##Property<Property, PropType>                                       \
    {                                                                                                                            \
    public:                                                                                                                      \
        enum_type##ArrayElementProperty(const Name& name = Name::none, const String& description = "",                           \
                                        const Name& group = Name::none, BitMask flags = 0)                                       \
            : enum_type##Property<Property, PropType>(name, description, nullptr, group, flags)                                  \
        {}                                                                                                                       \
    }

    declare_property_type(byte, Byte);
    declare_property_type(signed_byte, SignedByte);
    declare_property_type(int16_t, Int16);
    declare_property_type(uint16_t, UnsignedInt16);
    declare_property_type(int_t, Int);
    declare_property_type(uint_t, UnsignedInt);
    declare_property_type(int64_t, Int64);
    declare_property_type(uint64_t, UnsignedInt64);
    declare_property_type(bool, Bool);
    declare_property_type(float, Float);
    declare_property_type(Vector2D, Vec2);
    declare_property_type(Vector3D, Vec3);
    declare_property_type(Vector4D, Vec4);
    declare_property_type(String, String);
    declare_property_type(Path, Path);


    template<typename Instance, typename PropType = byte>
    using UnsignedInt8Property = ByteProperty<Instance, PropType>;

    template<typename Instance, typename PropType = int_t>
    using Int8Property = SignedByteProperty<Instance, PropType>;

    template<typename Instance, typename PropType = byte>
    using UnsignedInt32Property = UnsignedIntProperty<Instance, PropType>;

    template<typename Instance, typename PropType = uint_t>
    using Int32Property = IntProperty<Instance, PropType>;

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

    template<typename PropType>
    class EnumArrayElementPropperty : public EnumProperty<Property, PropType>
    {
    public:
        EnumArrayElementPropperty(class Enum* _enum, BitMask flags = 0, const Name& name = Name::none,
                                  const String& description = "", const Name& group = Name::none)
            : EnumProperty<Property, PropType>(name, description, nullptr, _enum, group, flags)
        {}
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

        FORCE_INLINE bool property_value(void* object, const PropertyValue& property_value) override
        {
            return false;
        }

        FORCE_INLINE Property::Type type() const override
        {
            return Property::Type::Object;
        }
    };

    template<typename Instance, typename StructType>
    class StructProperty : public BaseTypedProperty<StructType>
    {
    private:
        class Struct* _M_struct;
        StructType Instance::*_M_address;

    public:
        static_assert(std::is_class_v<StructType>, "PropType must be struct or class");

        StructProperty(const Name& name, const String& description, StructType Instance::*address, class Struct* struct_class,
                       const Name& group = Name::none, BitMask flags = 0)
            : BaseTypedProperty<StructType>(name, description, group, flags)
        {
            _M_struct  = struct_class;
            _M_address = address;
        }

        FORCE_INLINE Property::Type type() const override
        {
            return Property::Type::Struct;
        }

        PropertyValue property_value(const void* _object) const override
        {
            if (!_object)
                return {};

            if (_M_address)
            {
                if constexpr (std::is_base_of_v<Object, Instance>)
                {
                    const Object* object = reinterpret_cast<const Object*>(_object);
                    if (!this->is_valid_object(Instance::static_class_instance(), object))
                        return {};
                }

                const Instance* instance = reinterpret_cast<const Instance*>(_object);
                const StructType& out    = (instance->*_M_address);
                const void* out_address  = &out;
                return const_cast<void*>(out_address);
            }
            else
            {
                return const_cast<void*>(_object);
            }
        }

        bool property_value(void* _object, const PropertyValue& property_value) override
        {
            if constexpr (std::is_const_v<StructType>)
            {
                return false;
            }
            else
            {
                if (this->flags()(Property::IsConst) || !property_value.has_value() || !_object)
                    return false;

                if (_M_address)
                {
                    if constexpr (std::is_base_of_v<Object, Instance>)
                    {
                        Object* object = reinterpret_cast<Object*>(_object);
                        if (!this->is_valid_object(Instance::static_class_instance(), object))
                            return false;
                    }
                    Instance* instance      = reinterpret_cast<Instance*>(_object);
                    StructType* new_struct  = reinterpret_cast<StructType*>(std::any_cast<void*>(property_value));
                    (instance->*_M_address) = *new_struct;
                }
                else
                {
                    StructType* new_struct = reinterpret_cast<StructType*>(std::any_cast<void*>(property_value));
                    (*reinterpret_cast<StructType*>(_object)) = *new_struct;
                }

                return true;
            }
        }

        void* property_class() const override
        {
            return _M_struct;
        }
    };

    template<typename StructType>
    class StructArrayElementProperty : public StructProperty<Property, StructType>
    {
    public:
        StructArrayElementProperty(class Struct* struct_class, BitMask flags = 0, const Name& name = Name::none,
                                   const String& description = "", const Name& group = Name::none)
            : StructProperty<Property, StructType>(name, description, nullptr, struct_class, group, flags)
        {}
    };

    class ENGINE_EXPORT ArrayPropertyInterface : public Property
    {
    public:
        using Property::Property;

        virtual void* at(void* object, Index index)    = 0;
        virtual size_t size(void* object)              = 0;
        virtual bool emplace_back(void* object)        = 0;
        virtual bool pop_back(void* object)            = 0;
        virtual bool insert(void* object, Index index) = 0;
        virtual bool erase(void* object, Index index)  = 0;
    };

    template<typename Instance, typename VectorType>
    class ArrayProperty : public ArrayPropertyInterface
    {
    private:
        VectorType Instance::*_M_vector;
        Property* _M_element_property;

    public:
        FORCE_INLINE ArrayProperty(const Name& name, const String& description, VectorType Instance::*vector,
                                   Property* element_property, const Name& group = Name::none, BitMask flags = 0)
            : ArrayPropertyInterface(name, description, group, flags)
        {
            _M_vector           = vector;
            _M_element_property = element_property;
        }

        FORCE_INLINE size_t element_size() const override
        {
            return sizeof(VectorType);
        }

        FORCE_INLINE size_t min_alignment() const override
        {
            return alignof(VectorType);
        }

        FORCE_INLINE bool is_const() const override
        {
            return flags()(IsConst) || std::is_const_v<VectorType>;
        }

        void* property_class() const override
        {
            return _M_element_property;
        }

        FORCE_INLINE bool is_valid_object(const void* object) const
        {
            if constexpr (std::is_base_of_v<class Object, Instance>)
            {
                return Property::is_valid_object(Instance::static_class_instance(), reinterpret_cast<const Object*>(object));
            }
            return object != nullptr;
        }

        FORCE_INLINE PropertyValue property_value(const void* object) const override
        {
            return {};
        }

        FORCE_INLINE bool property_value(void* object, const PropertyValue& property_value) override
        {
            return false;
        }

        FORCE_INLINE Property::Type type() const override
        {
            return Property::Type::Array;
        }

        FORCE_INLINE void* at(void* object, Index index) override
        {
            if (!is_valid_object(object))
                return nullptr;
            return &(reinterpret_cast<Instance*>(object)->*_M_vector)[index];
        }

        size_t size(void* object) override
        {
            if (!is_valid_object(object))
                return 0;
            return (reinterpret_cast<Instance*>(object)->*_M_vector).size();
        }

        bool emplace_back(void* object) override
        {
            if (!is_valid_object(object))
                return false;
            (reinterpret_cast<Instance*>(object)->*_M_vector).emplace_back();
            return true;
        }

        bool pop_back(void* object) override
        {
            if (!is_valid_object(object))
                return false;
            (reinterpret_cast<Instance*>(object)->*_M_vector).pop_back();
            return true;
        }

        bool insert(void* object, Index index) override
        {
            if (!is_valid_object(object))
                return false;
            auto& vector = (reinterpret_cast<Instance*>(object)->*_M_vector);
            vector.emplace(vector.begin() + index);
            return true;
        }

        bool erase(void* object, Index index) override
        {
            if (!is_valid_object(object))
                return false;
            auto& vector = (reinterpret_cast<Instance*>(object)->*_M_vector);
            vector.erase(vector.begin() + index);
            return true;
        }

        ~ArrayProperty()
        {
            delete _M_element_property;
        }
    };
}// namespace Engine
