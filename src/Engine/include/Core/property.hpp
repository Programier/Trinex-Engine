#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/any.hpp>
#include <Core/flags.hpp>
#include <Core/name.hpp>

namespace Engine
{
    /*
        NOTE! If pointer to field in property class is nullptr, than use object address as address of property!
    */

    class Struct;
    enum class PropertyType
    {
        Undefined       = 0,
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
        Enum            = 16,
        Color3          = 17,
        Color4          = 18,
        LastPrimitive   = Color4,
        String          = 19,
        Path            = 20,
        Object          = 21,
        ObjectReference = 22,
        Struct          = 23,
        Array           = 24,
    };

    struct ENGINE_EXPORT ArrayPropertyValue final {
        const void* instace          = nullptr;
        class Property* element_type = nullptr;

        ArrayPropertyValue(const void* _instance = nullptr, class Property* element_type = nullptr);
    };

    struct ENGINE_EXPORT StructPropertyValue final {
        const void* instace     = nullptr;
        Struct* struct_instance = nullptr;

        StructPropertyValue(const void* _instance = nullptr, Struct* _struct = nullptr);
    };

#define declare_prop_constructor(type)                                                                                           \
    PropertyValue(const type&);                                                                                                  \
    PropertyValue& operator=(const type&);

    class ENGINE_EXPORT PropertyValue : public Any
    {
    private:
        PropertyType m_type = PropertyType::Undefined;

    public:
        PropertyValue();

        template<typename T>
        PropertyValue(T&& value, PropertyType type) : Any(std::move(value))
        {
            m_type = type;
        }

        PropertyValue(const PropertyValue&);
        PropertyValue(PropertyValue&&);
        PropertyValue& operator=(const PropertyValue&);
        PropertyValue& operator=(PropertyValue&&);

        declare_prop_constructor(byte);
        declare_prop_constructor(signed_byte);
        declare_prop_constructor(int16_t);
        declare_prop_constructor(uint16_t);
        declare_prop_constructor(int32_t);
        declare_prop_constructor(uint32_t);
        declare_prop_constructor(int64_t);
        declare_prop_constructor(uint64_t);
        declare_prop_constructor(bool);
        declare_prop_constructor(float);
        declare_prop_constructor(Vector2D);
        declare_prop_constructor(Vector3D);
        declare_prop_constructor(Vector4D);
        declare_prop_constructor(String);
        declare_prop_constructor(Path);
        declare_prop_constructor(ArrayPropertyValue);
        declare_prop_constructor(StructPropertyValue);


        byte byte_v() const;
        signed_byte signed_byte_v() const;
        int8_t int8_v() const;
        uint8_t uint8_v() const;
        int16_t int16_v() const;
        uint16_t uint16_v() const;
        int32_t int32_v() const;
        uint32_t uint32_v() const;
        int64_t int64_v() const;
        uint64_t uint64_v() const;
        bool bool_v() const;
        float float_v() const;
        Vector2D vec2_v() const;
        Vector3D vec3_v() const;
        Vector4D vec4_v() const;
        String string_v() const;
        Path path_v() const;
        EnumerateType enum_v() const;
        Object* object_v() const;
        Object* object_referece_v() const;
        StructPropertyValue struct_v() const;
        ArrayPropertyValue array_v() const;

        PropertyType type() const;
    };

#undef declare_prop_constructor

    class ENGINE_EXPORT Property
    {
    public:
        enum Flag
        {
            IsPrivate     = (1 << 0),
            IsConst       = (1 << 1),
            IsNativeConst = (1 << 2),
        };

    protected:
        Name m_name;
        Name m_group;
        String m_description;
        Flags<Property::Flag> m_flags;

        bool serialize_properies(class Struct* self, void* object, Archive& ar);

    public:
        CallBacks<void(void* object)> on_prop_changed;

        Property(const Name& name, const String& description, const Name& group = Name::none, BitMask flags = 0);

        const Name& name() const;
        const Name& group() const;
        const String& description() const;
        const Flags<Property::Flag>& flags() const;

        virtual void* prop_address(void* object)                                       = 0;
        virtual const void* prop_address(const void* object) const                     = 0;
        virtual PropertyValue property_value(const void* object) const                 = 0;
        virtual bool property_value(void* object, const PropertyValue& property_value) = 0;
        virtual size_t size() const                                                    = 0;
        virtual size_t min_alignment() const                                           = 0;
        virtual PropertyType type() const                                              = 0;
        virtual Struct* struct_instance();
        virtual class Enum* enum_instance();
        bool is_const() const;
        bool is_private() const;
        virtual bool archive_process(void* object, Archive& ar);

        virtual ~Property();
    };

    template<typename InstanceType, typename DataType>
    class PrimitivePropertyBase : public Property
    {
    protected:
        DataType InstanceType::*m_prop = nullptr;

    public:
        PrimitivePropertyBase(const Name& name, const String& description, const Name& group = Name::none, BitMask flags = 0)
            : Property(name, description, group, flags)
        {}

        void* prop_address(void* object) override
        {
            if (m_prop == nullptr)
                return object;

            if (object == nullptr)
                return nullptr;
            return &(reinterpret_cast<InstanceType*>(object)->*m_prop);
        }

        const void* prop_address(const void* object) const override
        {
            if (m_prop == nullptr)
                return object;

            if (object == nullptr)
                return nullptr;
            return &(reinterpret_cast<const InstanceType*>(object)->*m_prop);
        }
    };

    template<typename InstanceType, typename DataType, typename CastType, PropertyType prop_type>
    class PrimitiveProperty : public PrimitivePropertyBase<InstanceType, DataType>
    {
    private:
        using Super = PrimitivePropertyBase<InstanceType, DataType>;

    public:
        using ElementType = DataType;

        PrimitiveProperty(const Name& name, const String& description, DataType InstanceType::*prop,
                          const Name& group = Name::none, BitMask flags = 0)
            : PrimitivePropertyBase<InstanceType, DataType>(name, description, group, flags)
        {
            Super::m_prop = prop;

            if constexpr (std::is_const_v<DataType>)
            {
                Super::m_flags(Super::IsNativeConst, true);
            }
            else
            {
                Super::m_flags(Super::IsNativeConst, false);
            }
        }

        PropertyValue property_value(const void* object) const override
        {
            if (object)
            {
                return PropertyValue(static_cast<CastType>(*reinterpret_cast<const DataType*>(Super::prop_address(object))),
                                     prop_type);
            }

            return {};
        }

        bool property_value(void* object, const PropertyValue& property_value) override
        {
            InstanceType* instance = reinterpret_cast<InstanceType*>(object);
            if (!Super::is_const() && instance && property_value.type() == prop_type)
            {
                (*reinterpret_cast<DataType*>(Super::prop_address(object))) =
                        static_cast<DataType>(property_value.cast<CastType>());
                Property::on_prop_changed(object);
            }
            return false;
        }

        size_t size() const override
        {
            return sizeof(DataType);
        }

        size_t min_alignment() const override
        {
            return alignof(DataType);
        }

        PropertyType type() const override
        {
            return prop_type;
        }
    };

    template<typename InstanceType>
    class ByteProperty : public PrimitiveProperty<InstanceType, byte, byte, PropertyType::Byte>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, byte, byte, PropertyType::Byte>;

        ByteProperty(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                     const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class SignedByteProperty : public PrimitiveProperty<InstanceType, signed_byte, signed_byte, PropertyType::SignedByte>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, signed_byte, signed_byte, PropertyType::SignedByte>;

        SignedByteProperty(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                           const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class Int8Property : public PrimitiveProperty<InstanceType, int8_t, int8_t, PropertyType::Int8>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, int8_t, int8_t, PropertyType::Int8>;

        Int8Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                     const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class UInt8Property : public PrimitiveProperty<InstanceType, uint8_t, uint8_t, PropertyType::UnsignedInt8>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, uint8_t, uint8_t, PropertyType::UnsignedInt8>;

        UInt8Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                      const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class Int16Property : public PrimitiveProperty<InstanceType, int16_t, int16_t, PropertyType::Int16>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, int16_t, int16_t, PropertyType::Int16>;

        Int16Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                      const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class UInt16Property : public PrimitiveProperty<InstanceType, uint16_t, uint16_t, PropertyType::UnsignedInt16>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, uint16_t, uint16_t, PropertyType::UnsignedInt16>;

        UInt16Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                       const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class IntProperty : public PrimitiveProperty<InstanceType, int32_t, int32_t, PropertyType::Int>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, int32_t, int32_t, PropertyType::Int>;

        IntProperty(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                    const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class UIntProperty : public PrimitiveProperty<InstanceType, uint32_t, uint32_t, PropertyType::UnsignedInt>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, uint32_t, uint32_t, PropertyType::UnsignedInt>;

        UIntProperty(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                     const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class Int32Property : public PrimitiveProperty<InstanceType, int32_t, int32_t, PropertyType::Int32>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, int32_t, int32_t, PropertyType::Int32>;

        Int32Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                      const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class UInt32Property : public PrimitiveProperty<InstanceType, uint32_t, uint32_t, PropertyType::UnsignedInt32>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, uint32_t, uint32_t, PropertyType::UnsignedInt32>;

        UInt32Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                       const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class Int64Property : public PrimitiveProperty<InstanceType, int64_t, int64_t, PropertyType::Int64>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, int64_t, int64_t, PropertyType::Int64>;

        Int64Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                      const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class UInt64Property : public PrimitiveProperty<InstanceType, uint64_t, uint64_t, PropertyType::UnsignedInt64>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, uint64_t, uint64_t, PropertyType::UnsignedInt64>;

        UInt64Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                       const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class BoolProperty : public PrimitiveProperty<InstanceType, bool, bool, PropertyType::Bool>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, bool, bool, PropertyType::Bool>;

        BoolProperty(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                     const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class FloatProperty : public PrimitiveProperty<InstanceType, float, float, PropertyType::Float>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, float, float, PropertyType::Float>;

        FloatProperty(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                      const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class Vec2Property : public PrimitiveProperty<InstanceType, Vector2D, Vector2D, PropertyType::Vec2>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, Vector2D, Vector2D, PropertyType::Vec2>;

        Vec2Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                     const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class Vec3Property : public PrimitiveProperty<InstanceType, Vector3D, Vector3D, PropertyType::Vec3>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, Vector3D, Vector3D, PropertyType::Vec3>;

        Vec3Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                     const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class Vec4Property : public PrimitiveProperty<InstanceType, Vector4D, Vector4D, PropertyType::Vec4>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, Vector4D, Vector4D, PropertyType::Vec4>;

        Vec4Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                     const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType, typename EnumType>
    class EnumProperty : public PrimitiveProperty<InstanceType, EnumType, EnumerateType, PropertyType::Enum>
    {
    private:
        class Enum* m_enum;

    public:
        EnumProperty(const Name& name, const String& description, EnumType InstanceType::*prop, class Enum* _enum,
                     const Name& group = Name::none, BitMask flags = 0)
            : PrimitiveProperty<InstanceType, EnumType, EnumerateType, PropertyType::Enum>(name, description, prop, group, flags)
        {
            m_enum = _enum;
        }

        class Enum* enum_instance() override
        {
            return m_enum;
        }

        size_t size() const override
        {
            return sizeof(EnumerateType);
        }

        size_t min_alignment() const override
        {
            return alignof(EnumerateType);
        }
    };

    template<typename InstanceType>
    class Color3Property : public PrimitiveProperty<InstanceType, Color3, Color3, PropertyType::Color3>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, Color3, Color3, PropertyType::Color3>;

        Color3Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                       const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class Color4Property : public PrimitiveProperty<InstanceType, Color4, Color4, PropertyType::Color4>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, Color4, Color4, PropertyType::Color4>;

        Color4Property(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                       const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class StringProperty : public PrimitiveProperty<InstanceType, String, String, PropertyType::String>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, String, String, PropertyType::String>;

        StringProperty(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                       const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType>
    class PathProperty : public PrimitiveProperty<InstanceType, Path, Path, PropertyType::Path>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, Path, Path, PropertyType::Path>;

        PathProperty(const Name& name, const String& description, Super::ElementType InstanceType::*prop,
                     const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}
    };

    template<typename InstanceType, typename ObjectType>
    class ObjectProperty : public PrimitiveProperty<InstanceType, ObjectType*, Object*, PropertyType::Object>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, ObjectType*, Object*, PropertyType::Object>;

        ObjectProperty(const Name& name, const String& description, ObjectType* InstanceType::*prop,
                       const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}

        Struct* struct_instance() override
        {
            return ObjectType::static_class_instance();
        }
    };

    template<typename InstanceType, typename ObjectType>
    class ObjectReferenceProperty : public PrimitiveProperty<InstanceType, ObjectType*, Object*, PropertyType::ObjectReference>
    {
    public:
        using Super = PrimitiveProperty<InstanceType, ObjectType*, Object*, PropertyType::ObjectReference>;

        ObjectReferenceProperty(const Name& name, const String& description, ObjectType* InstanceType::*prop,
                                const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, prop, group, flags)
        {}

        Struct* struct_instance() override
        {
            return ObjectType::static_class_instance();
        }
    };

    template<typename InstanceType, typename StructType>
    class StructProperty : public PrimitivePropertyBase<InstanceType, StructType>
    {
    private:
        Struct* m_struct = nullptr;

        using Super = PrimitivePropertyBase<InstanceType, StructType>;

    public:
        StructProperty(const Name& name, const String& description, StructType InstanceType::*prop, class Struct* _struct,
                       const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, group, flags)
        {
            Super::m_prop = prop;
            m_struct      = _struct;

            if constexpr (std::is_const_v<StructType>)
            {
                Super::m_flags(Super::IsConst, true);
            }
            else
            {
                Super::m_flags(Super::IsNativeConst, true);
            }
        }

        PropertyValue property_value(const void* object) const override
        {
            const void* instance = Super::prop_address(object);

            if (instance)
            {
                return PropertyValue(StructPropertyValue(instance, m_struct), PropertyType::Struct);
            }

            return {};
        }

        bool property_value(void* object, const PropertyValue& property_value) override
        {
            InstanceType* instance = reinterpret_cast<InstanceType*>(object);
            if (instance == nullptr || Super::is_const())
                return false;

            StructPropertyValue value = property_value.struct_v();
            if (value.struct_instance != m_struct)
                return false;


            (*reinterpret_cast<StructType*>(Super::prop_address(object))) = *reinterpret_cast<const StructType*>(value.instace);
            Property::on_prop_changed(object);
            return true;
        }

        size_t size() const override
        {
            return sizeof(StructType);
        }

        size_t min_alignment() const override
        {
            return alignof(StructType);
        }

        PropertyType type() const override
        {
            return PropertyType::Struct;
        }

        bool archive_process(void* object, Archive& ar) override
        {
            object = Super::prop_address(object);
            if (object)
            {
                return Super::serialize_properies(m_struct, object, ar);
            }
            return false;
        }

        Struct* struct_instance() override
        {
            return m_struct;
        }
    };

    class ENGINE_EXPORT ArrayPropertyInterface : public Property
    {
    public:
        using Property::Property;

        virtual Property* element_type() const             = 0;
        virtual void* at(void* object, Index index)        = 0;
        virtual size_t elements_count(void* object)        = 0;
        virtual bool emplace_back(void* object)            = 0;
        virtual bool pop_back(void* object)                = 0;
        virtual bool insert(void* object, Index index)     = 0;
        virtual bool erase(void* object, Index index)      = 0;
        virtual void resize(void* object, size_t new_size) = 0;

        bool archive_process(void* object, Archive& ar) override;
    };

    template<typename InstanceType, typename ArrayType>
    class ArrayProperty : public ArrayPropertyInterface
    {
    private:
        ArrayType InstanceType::*m_prop;
        Property* m_element_property;

        using Super = ArrayPropertyInterface;

    public:
        ArrayProperty(const Name& name, const String& description, ArrayType InstanceType::*prop,
                      class Property* element_property, const Name& group = Name::none, BitMask flags = 0)
            : Super(name, description, group, flags)
        {
            m_prop             = prop;
            m_element_property = element_property;
        }

        Property* element_type() const override
        {
            return m_element_property;
        }

        void* at(void* object, Index index) override
        {
            ArrayType* array = array_from(object);
            if (array)
            {
                return &((*array)[index]);
            }

            return nullptr;
        }

        size_t elements_count(void* object) override
        {
            ArrayType* array = array_from(object);
            if (array)
            {
                return array->size();
            }

            return 0;
        }

        bool emplace_back(void* object) override
        {
            ArrayType* array = array_from(object);

            if (array)
            {
                array->emplace_back();
                return true;
            }

            return false;
        }

        bool pop_back(void* object) override
        {
            ArrayType* array = array_from(object);

            if (array)
            {
                array->pop_back();
                return true;
            }

            return false;
        }

        bool insert(void* object, Index index) override
        {
            ArrayType* array = array_from(object);
            if (array == nullptr)
                return false;

            array->emplace(array->begin() + index);
            return true;
        }

        bool erase(void* object, Index index) override
        {
            ArrayType* array = array_from(object);
            if (array == nullptr)
                return false;

            array->erase(array->begin() + index);
            return true;
        }

        void resize(void* object, size_t new_size) override
        {
            ArrayType* array = array_from(object);

            if (array)
            {
                array->resize(new_size);
            }
        }

        void* prop_address(void* object) override
        {
            if (m_prop == nullptr)
                return object;

            if (object == nullptr)
                return nullptr;
            return &(reinterpret_cast<InstanceType*>(object)->*m_prop);
        }

        const void* prop_address(const void* object) const override
        {
            if (m_prop == nullptr)
                return object;

            if (object == nullptr)
                return nullptr;
            return &(reinterpret_cast<const InstanceType*>(object)->*m_prop);
        }

        ArrayType* array_from(void* object)
        {
            return reinterpret_cast<ArrayType*>(prop_address(object));
        }

        const ArrayType* array_from(void* object) const
        {
            return reinterpret_cast<ArrayType*>(prop_address(object));
        }

        PropertyValue property_value(const void* object) const override
        {
            auto array = prop_address(object);
            if (array)
            {
                return PropertyValue(ArrayPropertyValue(array, m_element_property), PropertyType::Array);
            }

            return {};
        }

        bool property_value(void* object, const PropertyValue& property_value) override
        {
            return false;
        }

        size_t size() const override
        {
            return sizeof(ArrayType);
        }

        size_t min_alignment() const override
        {
            return alignof(ArrayType);
        }

        PropertyType type() const override
        {
            return PropertyType::Array;
        }

        ~ArrayProperty()
        {
            delete m_element_property;
        }
    };
}// namespace Engine
