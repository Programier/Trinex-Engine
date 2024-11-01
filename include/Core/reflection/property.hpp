#pragma once
#include <Core/reflection/object.hpp>
#include <type_traits>

namespace Engine
{
	class Object;
}

namespace Engine::Refl
{
#define trinex_refl_prop_type_filter(...)                                                                                        \
public:                                                                                                                          \
	template<typename T>                                                                                                         \
	static constexpr inline bool is_supported = !std::is_const_v<std::remove_pointer_t<T>> && __VA_ARGS__;                       \
																																 \
private:

	template<typename Decl>
	struct PropertySignatureParser {
	};

	template<typename T, typename Instance>
	struct PropertySignatureParser<T Instance::*> {
		using Type  = T;
		using Owner = Instance;
	};

	template<auto prop>
	struct PropertyParser : public PropertySignatureParser<decltype(prop)> {
	};

	template<auto prop, typename T>
	struct NativePropertyTyped {
	};

	template<auto prop>
	struct NativeProperty : public NativePropertyTyped<prop, typename PropertyParser<prop>::Type> {
		using Super = NativePropertyTyped<prop, typename PropertyParser<prop>::Type>;
		using Super::Super;
	};

	class ENGINE_EXPORT Property : public Object
	{
		declare_reflect_type(Property, Object);

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

	private:
		const BitMask m_flags = 0;

	public:
		Property(BitMask flags = 0);

		bool is_const() const;
		bool is_private() const;
		bool is_serializable() const;
		bool is_hidden() const;
		bool is_color() const;

		virtual void* address(void* context)                    = 0;
		virtual const void* address(const void* context) const  = 0;
		virtual size_t size() const                             = 0;
		virtual bool archive_process(void* object, Archive& ar) = 0;
		virtual String script_type_name() const                 = 0;

		template<typename T>
		T* address_as(void* context)
		{
			return reinterpret_cast<T*>(address(context));
		}

		template<typename T>
		const T* address_as(const void* context) const
		{
			return reinterpret_cast<const T*>(address(context));
		}

		virtual ~Property();
	};

	class ENGINE_EXPORT PrimitiveProperty : public Property
	{
		declare_reflect_type(PrimitiveProperty, Property);

	public:
		using Property::Property;
		bool archive_process(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT BooleanProperty : public PrimitiveProperty
	{
		declare_reflect_type(BooleanProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(std::is_same_v<T, bool>);

	public:
		String script_type_name() const override;
	};

	class ENGINE_EXPORT IntegerProperty : public PrimitiveProperty
	{
		declare_reflect_type(IntegerProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(!std::is_same_v<T, bool> && std::is_integral_v<T> && !std::is_enum_v<T>);

	public:
		String script_type_name() const override;
		bool is_unsigned() const;
		virtual bool is_signed() const = 0;
	};

	class ENGINE_EXPORT FloatProperty : public PrimitiveProperty
	{
		declare_reflect_type(FloatProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(std::is_floating_point_v<T>);

	public:
		String script_type_name() const override;
	};

	class ENGINE_EXPORT VectorProperty : public PrimitiveProperty
	{
		declare_reflect_type(VectorProperty, PrimitiveProperty);

	private:
		template<typename T>
		struct IsVector : std::false_type {
		};

		template<glm::length_t L, typename T, glm::qualifier Q>
		struct IsVector<glm::vec<L, T, Q>> : std::true_type {
		};

		trinex_refl_prop_type_filter(IsVector<T>::value);

	public:
		String script_type_name() const override;

		virtual size_t length() const                              = 0;
		virtual Property* element_property(size_t index = 0) const = 0;
		virtual size_t element_size() const                        = 0;

		virtual void* element_address(void* context, size_t index, bool is_vector_context = false)                   = 0;
		virtual const void* element_address(const void* context, size_t index, bool is_vector_context = false) const = 0;

		template<typename T>
		T* element_address_as(void* context, size_t index, bool is_vector_context = false)
		{
			return reinterpret_cast<T*>(element_address(context, index, is_vector_context));
		}

		template<typename T>
		const T* element_address_as(const void* context, size_t index, bool is_vector_context = false) const
		{
			return reinterpret_cast<const T*>(element_address(context, index, is_vector_context));
		}
	};

	class ENGINE_EXPORT EnumProperty : public PrimitiveProperty
	{
		declare_reflect_type(EnumProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(std::is_enum_v<T>);

	private:
		class Enum* m_enum;

	public:
		EnumProperty(Enum* enum_instance, BitMask flags = 0);
		Enum* enum_instance() const;
		String script_type_name() const override;
	};

	class ENGINE_EXPORT StringProperty : public Property
	{
		declare_reflect_type(StringProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<String, T>);

	public:
		using Property::Property;

		bool archive_process(void* object, Archive& ar) override;
		String script_type_name() const override;
	};

	class ENGINE_EXPORT NameProperty : public Property
	{
		declare_reflect_type(NameProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<Name, T>);

	public:
		using Property::Property;

		bool archive_process(void* object, Archive& ar) override;
		String script_type_name() const override;
	};

	class ENGINE_EXPORT PathProperty : public Property
	{
		declare_reflect_type(PathProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<Path, T>);

	public:
		using Property::Property;

		bool archive_process(void* object, Archive& ar) override;
		String script_type_name() const override;
	};

	class ENGINE_EXPORT ObjectProperty : public Property
	{
		declare_reflect_type(ObjectProperty, Property);
		trinex_refl_prop_type_filter(std::is_pointer_v<T>&& std::is_base_of_v<Engine::Object, std::remove_pointer_t<T>>);

	private:
		bool m_inline_serialization = false;

	public:
		using Property::Property;

		bool archive_process(void* object, Archive& ar) override;
		String script_type_name() const override;

		Engine::Object* object(void* context);
		const Engine::Object* object(const void* context) const;
		bool inline_serialization() const;
		ObjectProperty& inline_serialization(bool flag);

		virtual Class* class_instance() const = 0;
	};

	class ENGINE_EXPORT StructProperty : public Property
	{
		declare_reflect_type(StructProperty, Property);
		trinex_refl_prop_type_filter(std::is_class_v<T>&& std::is_same_v<decltype(T::static_struct_instance()), Refl::Struct*>);

	public:
		using Property::Property;

		bool archive_process(void* object, Archive& ar) override;
		String script_type_name() const override;

		virtual Struct* struct_instance() const = 0;
	};

	class ENGINE_EXPORT ArrayProperty : public Property
	{
		declare_reflect_type(ArrayProperty, Property);

	protected:
		template<typename T>
		struct IsArray : std::false_type {
		};

		template<typename T, typename Alloc>
		struct IsArray<Engine::Containers::Vector<T, Alloc>> : std::true_type {
		};

		template<typename T>
		struct IsScriptableArray : std::false_type {
		};

		template<typename T>
		struct IsScriptableArray<Engine::Vector<T>> : std::true_type {
		};

		trinex_refl_prop_type_filter(IsArray<T>::value);

	public:
		using Property::Property;

		bool archive_process(void* object, Archive& ar) override;
		String script_type_name() const override;

		virtual Property* element_property() const = 0;
		virtual size_t element_size() const        = 0;

		virtual void* element_address(void* context, size_t index, bool is_vector_context = false)                   = 0;
		virtual const void* element_address(const void* context, size_t index, bool is_vector_context = false) const = 0;

		virtual size_t length(const void* context, bool is_vector_context = false) const                             = 0;
		virtual ArrayProperty& emplace_back(void* context, bool is_vector_context = false)                           = 0;
		virtual ArrayProperty& pop_back(void* context, bool is_vector_context = false)                               = 0;
		virtual ArrayProperty& insert(void* context, size_t index, size_t count = 1, bool is_vector_context = false) = 0;
		virtual ArrayProperty& erase(void* context, size_t index, size_t count = 1, bool is_vector_context = false)  = 0;
		virtual ArrayProperty& resize(void* context, size_t new_size, bool is_vector_context = false)                = 0;

		template<typename T>
		T* element_address_as(void* context, size_t index, bool is_vector_context = false)
		{
			return reinterpret_cast<T*>(element_address(context, index, is_vector_context));
		}

		template<typename T>
		const T* element_address_as(const void* context, size_t index, bool is_vector_context = false) const
		{
			return reinterpret_cast<const T*>(element_address(context, index, is_vector_context));
		}
	};

	//////////////////// SPECIALIZATIONS ////////////////////

	template<auto prop, typename Super, typename = std::enable_if_t<std::is_base_of_v<Property, Super>>>
	class TypedProperty : public Super
	{
	public:
		using Type     = typename PropertyParser<prop>::Type;
		using Instance = typename PropertyParser<prop>::Owner;

		using Super::Super;

		void* address(void* context) override
		{
			if (prop == nullptr)
				return context;

			Instance* instance = reinterpret_cast<Instance*>(context);
			return &(instance->*prop);
		}

		const void* address(const void* context) const override
		{
			if (prop == nullptr)
				return context;

			const Instance* instance = reinterpret_cast<const Instance*>(context);
			return &(instance->*prop);
		}

		size_t size() const override
		{
			return sizeof(Type);
		}
	};

	template<auto prop, typename T>
		requires(BooleanProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, BooleanProperty> {
		using Super = TypedProperty<prop, BooleanProperty>;
		using Super::Super;
	};

	template<auto prop, typename T>
		requires(IntegerProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, IntegerProperty> {
		using Super = TypedProperty<prop, IntegerProperty>;
		using Super::Super;

		bool is_signed() const override
		{
			return std::is_signed_v<T>;
		}
	};

	template<auto prop, typename T>
		requires(FloatProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, FloatProperty> {
		using Super = TypedProperty<prop, FloatProperty>;
		using Super::Super;
	};

	template<auto prop, typename T>
		requires(VectorProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, VectorProperty> {
	private:
		static Property** construct_element_properties()
		{
			constexpr size_t len = T::length();
			static_assert(len > 0, "Lenght of the vector must be greater than 0");
			static Property* properties[len];

			constexpr Object* owner = nullptr;

			if constexpr (len >= 1)
				properties[0] = Object::new_instance<NativeProperty<&T::x>>(owner, "X");
			if constexpr (len >= 2)
				properties[1] = Object::new_instance<NativeProperty<&T::y>>(owner, "Y");
			if constexpr (len >= 3)
				properties[2] = Object::new_instance<NativeProperty<&T::z>>(owner, "Z");
			if constexpr (len >= 4)
				properties[3] = Object::new_instance<NativeProperty<&T::w>>(owner, "W");

			return properties;
		}

	public:
		using Super = TypedProperty<prop, VectorProperty>;
		using Super::Super;

		size_t length() const override
		{
			return T::length();
		}

		Property* element_property(size_t index = 0) const override
		{
			if (index >= static_cast<size_t>(T::length()))
				return nullptr;

			static Property** properties = construct_element_properties();
			return properties[index];
		}

		size_t element_size() const override
		{
			return sizeof(typename T::value_type);
		}

		void* element_address(void* context, size_t index, bool is_vector_context = false) override
		{
			if (index >= static_cast<size_t>(T::length()))
				return nullptr;

			if (!is_vector_context)
				context = this->address(context);

			return reinterpret_cast<byte*>(context) + sizeof(typename T::value_type) * index;
		}

		const void* element_address(const void* context, size_t index, bool is_vector_context = false) const override
		{
			if (index >= length())
				return nullptr;

			if (!is_vector_context)
				context = this->address(context);

			return reinterpret_cast<const byte*>(context) + sizeof(typename T::value_type) * index;
		}
	};

	template<auto prop, typename T>
		requires(EnumProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, EnumProperty> {
		using Super = TypedProperty<prop, EnumProperty>;
		using Super::Super;
	};

	template<auto prop, typename T>
		requires(StringProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, StringProperty> {
		using Super = TypedProperty<prop, StringProperty>;
		using Super::Super;
	};

	template<auto prop, typename T>
		requires(NameProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, NameProperty> {
		using Super = TypedProperty<prop, NameProperty>;
		using Super::Super;
	};

	template<auto prop, typename T>
		requires(PathProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, PathProperty> {
		using Super = TypedProperty<prop, PathProperty>;
		using Super::Super;
	};

	template<auto prop, typename T>
		requires(ObjectProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, ObjectProperty> {
		using Super = TypedProperty<prop, ObjectProperty>;
		using Super::Super;

		Class* class_instance() const override
		{
			return std::remove_pointer_t<T>::static_class_instance();
		}
	};


	template<auto prop, typename T>
		requires(ArrayProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, ArrayProperty> {
		using Super = TypedProperty<prop, ArrayProperty>;
		using Super::Super;
		using Value = typename T::value_type;

		Property* element_property() const override
		{
			constexpr Value NativePropertyTyped::*null_prop = nullptr;
			static Property* instance = Object::new_instance<NativeProperty<null_prop>>(nullptr, StringView("Element"));
			return instance;
		}

		size_t element_size() const override
		{
			return sizeof(Value);
		}

		T& array_of(void* context, bool is_vector_context)
		{
			if (!is_vector_context)
				context = this->address(context);
			return *reinterpret_cast<T*>(context);
		}

		const T& array_of(const void* context, bool is_vector_context) const
		{
			if (!is_vector_context)
				context = this->address(context);
			return *reinterpret_cast<const T*>(context);
		}

		void* element_address(void* context, size_t index, bool is_vector_context = false) override
		{
			T& array = array_of(context, is_vector_context);

			if (array.size() <= index)
				return nullptr;

			return array.data() + index;
		}

		const void* element_address(const void* context, size_t index, bool is_vector_context = false) const override
		{
			const T& array = array_of(context, is_vector_context);

			if (array.size() <= index)
				return nullptr;

			return array.data() + index;
		}

		size_t length(const void* context, bool is_vector_context = false) const override
		{
			return array_of(context, is_vector_context).size();
		}

		ArrayProperty& emplace_back(void* context, bool is_vector_context = false) override
		{
			array_of(context, is_vector_context).emplace_back();
			return *this;
		}

		ArrayProperty& pop_back(void* context, bool is_vector_context = false) override
		{
			array_of(context, is_vector_context).pop_back();
			return *this;
		}

		ArrayProperty& insert(void* context, size_t index, size_t count = 1, bool is_vector_context = false) override
		{
			T& array = array_of(context, is_vector_context);
			array.insert(array.begin() + index, count, Value());
			return *this;
		}

		ArrayProperty& erase(void* context, size_t index, size_t count = 1, bool is_vector_context = false) override
		{
			T& array = array_of(context, is_vector_context);
			array.erase(array.begin() + index, array.begin() + index + count);
			return *this;
		}

		ArrayProperty& resize(void* context, size_t new_size, bool is_vector_context = false) override
		{
			array_of(context, is_vector_context).resize(new_size, Value());
			return *this;
		}
	};

#undef trinex_refl_prop_type_filter
#define trinex_refl_prop(self, class_name, prop_name)                                                                            \
	self->new_child<Engine::Refl::NativeProperty<&class_name::prop_name>>(#prop_name)
}// namespace Engine::Refl
