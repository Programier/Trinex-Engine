#pragma once
#include <Core/archive.hpp>
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/any.hpp>
#include <Core/etl/type_info.hpp>
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
		Enum            = 15,
		LastPrimitive   = Enum,
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

		template<typename T>
		PropertyValue(T&& value) : Any(std::forward<T>(value))
		{}

		PropertyValue(const PropertyValue&);
		PropertyValue(PropertyValue&&);
		PropertyValue& operator=(const PropertyValue&);
		PropertyValue& operator=(PropertyValue&&);

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

		operator bool() const;
	};

#undef declare_prop_constructor

	class ENGINE_EXPORT Property
	{
	public:
		enum Flag
		{
			IsPrivate         = BIT(0),
			IsConst           = BIT(1),
			IsNativeConst     = BIT(2),
			IsNotSerializable = BIT(3),
			IsHidden          = BIT(4),
			IsColor           = BIT(5),
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

		Property& name(const Name& new_name);
		const Name& name() const;
		const Name& group() const;
		const String& description() const;
		const Flags<Property::Flag>& flags() const;

		bool is_const() const;
		bool is_private() const;
		bool is_serializable() const;
		bool is_hidden() const;
		bool is_color() const;

		virtual void* prop_address(void* object)                                       = 0;
		virtual const void* prop_address(const void* object) const                     = 0;
		virtual PropertyValue property_value(const void* object) const                 = 0;
		virtual bool property_value(void* object, const PropertyValue& property_value) = 0;
		virtual size_t size() const                                                    = 0;
		virtual size_t min_alignment() const                                           = 0;
		virtual PropertyType type() const                                              = 0;
		virtual size_t type_id() const;

		virtual Struct* struct_instance();
		virtual class Enum* enum_instance();

		virtual bool archive_process(void* object, Archive& ar);

		virtual ~Property();
	};

	template<typename Instance, typename Type>
	struct ClassProperty : public Property {
		Type Instance::*m_prop = nullptr;

	public:
		ClassProperty(const Name& name, const String& description, Type Instance::*prop, const Name& group = Name::none,
		              BitMask flags = 0)
		    : Property(name, description, group, flags)
		{
			m_prop = prop;
		}

		void* prop_address(void* object) override
		{
			if (m_prop == nullptr)
				return object;

			if (object == nullptr)
				return nullptr;
			return &(reinterpret_cast<Instance*>(object)->*m_prop);
		}

		const void* prop_address(const void* object) const override
		{
			if (m_prop == nullptr)
				return object;

			if (object == nullptr)
				return nullptr;
			return &(reinterpret_cast<const Instance*>(object)->*m_prop);
		}

		PropertyValue property_value(const void* object) const override
		{
			if (object)
			{
				return PropertyValue(*reinterpret_cast<const Type*>(prop_address(object)));
			}

			return {};
		}

		bool property_value(void* object, const PropertyValue& property_value) override
		{
			Instance* instance = reinterpret_cast<Instance*>(object);
			if (!is_const() && instance /*&& property_value.type() == prop_type*/)
			{
				(*reinterpret_cast<Type*>(prop_address(object))) = static_cast<Type>(property_value.cast<Type>());
				Property::on_prop_changed(object);
				return true;
			}
			return false;
		}

		size_t size() const override
		{
			return sizeof(Type);
		}

		size_t min_alignment() const override
		{
			return alignof(Type);
		}

		PropertyType type() const override
		{
			return PropertyType::Undefined;
		}

		size_t type_id() const override
		{
			return Engine::type_info<Type>::id();
		}

		bool archive_process(void* object, Archive& ar) override
		{
			Type* address = reinterpret_cast<Type*>(prop_address(object));
			return ar & (*address);
		}
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
			Super::m_flags(Super::IsNativeConst, std::is_const_v<DataType>);
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
				return true;
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

			Super::m_flags(Super::IsNativeConst, std::is_const_v<StructType>);
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

	ENGINE_EXPORT Name default_array_element_name(class ArrayPropertyInterface* interface, void* object, size_t index);
	ENGINE_EXPORT Name default_array_object_element_name(class ArrayPropertyInterface* interface, void* object, size_t index);
	using ArrayPropertyElementNameCallback = Name (*)(class ArrayPropertyInterface*, void*, size_t);

	class ENGINE_EXPORT ArrayPropertyInterface : public Property
	{
	private:
		ArrayPropertyElementNameCallback m_element_name_callback;

	public:
		ArrayPropertyInterface(const Name& name, const String& description, const Name& group = Name::none, BitMask flags = 0);

		virtual Property* element_type() const                  = 0;
		virtual void* at(void* object, Index index)             = 0;
		virtual size_t elements_count(const void* object) const = 0;
		virtual bool emplace_back(void* object)                 = 0;
		virtual bool pop_back(void* object)                     = 0;
		virtual bool insert(void* object, Index index)          = 0;
		virtual bool erase(void* object, Index index)           = 0;
		virtual void resize(void* object, size_t new_size)      = 0;

		bool archive_process(void* object, Archive& ar) override;

		ArrayPropertyElementNameCallback element_name_callback() const;
		ArrayPropertyInterface& element_name_callback(ArrayPropertyElementNameCallback callback);
		Name element_name(void* object, size_t index);
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

		size_t elements_count(const void* object) const override
		{
			const ArrayType* array = array_from(object);
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

		const ArrayType* array_from(const void* object) const
		{
			return reinterpret_cast<const ArrayType*>(prop_address(object));
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
