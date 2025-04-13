#pragma once
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/reflection/object.hpp>

namespace Engine
{
	class Object;
	class ScriptFunction;
	class Path;
}// namespace Engine

namespace Engine::Refl
{
	template<typename T>
	class SubClassOf;

	namespace PropertyChangeType
	{
		using Type = BitMask;

		constexpr inline const Type unspecified   = BIT(0);
		constexpr inline const Type array_add     = BIT(1);
		constexpr inline const Type array_remove  = BIT(2);
		constexpr inline const Type array_clear   = BIT(3);
		constexpr inline const Type value_set     = BIT(4);
		constexpr inline const Type duplicate     = BIT(5);
		constexpr inline const Type interactive   = BIT(6);
		constexpr inline const Type member_change = BIT(7);
	};// namespace PropertyChangeType

	struct PropertyChangedEvent {
		void* context;
		PropertyChangeType::Type type;
		Property* property;
		Property* member_property;

		PropertyChangedEvent(void* context, PropertyChangeType::Type type, Property* property)
			: context(context), type(type), property(property), member_property(property)
		{}

		template<typename T>
		T* context_as() const
		{
			return reinterpret_cast<T*>(context);
		}
	};


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
			IsReadOnly               = BIT(0),
			IsTransient              = BIT(1),
			IsHidden                 = BIT(2),
			IsColor                  = BIT(3),
			InlineSingleFieldStructs = BIT(4),
		};

		using ChangeListener = Function<void(const PropertyChangedEvent&)>;

	private:
		CallBacks<void(const PropertyChangedEvent&)> m_change_listeners;

	protected:
		BitMask m_flags = 0;

		template<size_t size>
		struct InnerProperties {
			Property* properties[size];
		};

		static void trigger_object_event(const PropertyChangedEvent& event);

		inline bool check_flag(BitMask mask) const { return (m_flags & mask) == mask; }

	public:
		Property(BitMask flags = 0);

		inline bool is_read_only() const { return check_flag(IsReadOnly); }
		inline bool is_transient() const { return check_flag(IsTransient); }
		inline bool is_hidden() const { return check_flag(IsHidden); }
		inline bool is_color() const { return check_flag(IsColor); }
		inline bool inline_single_field_structs() const { return check_flag(InlineSingleFieldStructs); }

		Identifier add_change_listener(const ChangeListener& listener);
		Property& push_change_listener(const ChangeListener& listener);
		Property& remove_change_listener(Identifier id);

		using Super::display_name;
		virtual void* address(void* context)                   = 0;
		virtual const void* address(const void* context) const = 0;
		virtual size_t size() const                            = 0;
		virtual bool serialize(void* object, Archive& ar)      = 0;
		virtual const String& property_name(const void* context);
		virtual Property& on_property_changed(const PropertyChangedEvent& event);

		static void register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast);

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
		bool serialize(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT BooleanProperty : public PrimitiveProperty
	{
		declare_reflect_type(BooleanProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(std::is_same_v<T, bool>);

	public:
		using PrimitiveProperty::PrimitiveProperty;
		size_t size() const override;
	};

	class ENGINE_EXPORT IntegerProperty : public PrimitiveProperty
	{
		declare_reflect_type(IntegerProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(!std::is_same_v<T, bool> && std::is_integral_v<T> && !std::is_enum_v<T>);

	public:
		using PrimitiveProperty::PrimitiveProperty;
		bool is_unsigned() const;
		virtual bool is_signed() const = 0;
	};

	class ENGINE_EXPORT FloatProperty : public PrimitiveProperty
	{
		declare_reflect_type(FloatProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(std::is_floating_point_v<T>);

	public:
		using PrimitiveProperty::PrimitiveProperty;
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
		using PrimitiveProperty::PrimitiveProperty;

		virtual size_t length() const              = 0;
		virtual Property* element_property() const = 0;
		virtual size_t element_size() const        = 0;

		virtual void* element_address(void* context, size_t index, bool is_matrix_context = false)                   = 0;
		virtual const void* element_address(const void* context, size_t index, bool is_matrix_context = false) const = 0;

		template<typename T>
		T* element_address_as(void* context, size_t index, bool is_matrix_context = false)
		{
			return reinterpret_cast<T*>(element_address(context, index, is_matrix_context));
		}

		template<typename T>
		const T* element_address_as(const void* context, size_t index, bool is_vector_context = false) const
		{
			return reinterpret_cast<const T*>(element_address(context, index, is_vector_context));
		}
	};

	class ENGINE_EXPORT MatrixProperty : public PrimitiveProperty
	{
		declare_reflect_type(MatrixProperty, PrimitiveProperty);

	private:
		template<typename T>
		struct IsMatrix : std::false_type {
		};

		template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
		struct IsMatrix<glm::mat<C, R, T, Q>> : std::true_type {
		};

		trinex_refl_prop_type_filter(IsMatrix<T>::value);

	public:
		using PrimitiveProperty::PrimitiveProperty;

		virtual size_t columns() const         = 0;
		virtual size_t rows() const            = 0;
		virtual Property* row_property() const = 0;
		virtual size_t row_size() const        = 0;

		virtual void* row_address(void* context, size_t index, bool is_vector_context = false)                   = 0;
		virtual const void* row_address(const void* context, size_t index, bool is_vector_context = false) const = 0;

		template<typename T>
		T* row_address_as(void* context, size_t index, bool is_vector_context = false)
		{
			return reinterpret_cast<T*>(row_address(context, index, is_vector_context));
		}

		template<typename T>
		const T* row_address_as(const void* context, size_t index, bool is_vector_context = false) const
		{
			return reinterpret_cast<const T*>(row_address(context, index, is_vector_context));
		}
	};

	class ENGINE_EXPORT EnumProperty : public PrimitiveProperty
	{
		declare_reflect_type(EnumProperty, PrimitiveProperty);

		template<typename T>
		using enum_detector =
				std::enable_if_t<std::is_enum_v<typename T::Enum> && T::is_enum && !T::is_bitfield_enum && T::is_enum_reflected>;
		trinex_refl_prop_type_filter(is_detected_v<enum_detector, T> && sizeof(T) <= sizeof(EnumerateType));

	public:
		using PrimitiveProperty::PrimitiveProperty;

		virtual Enum* enum_instance() const = 0;
		EnumerateType value(const void* context) const;
		EnumProperty& value(void* context, EnumerateType value);
	};

	class ENGINE_EXPORT StringProperty : public Property
	{
		declare_reflect_type(StringProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<String, T>);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;
		size_t size() const override;
	};

	class ENGINE_EXPORT NameProperty : public Property
	{
		declare_reflect_type(NameProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<Name, T>);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT PathProperty : public Property
	{
		declare_reflect_type(PathProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<Path, T>);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT ObjectProperty : public Property
	{
		declare_reflect_type(ObjectProperty, Property);
		trinex_refl_prop_type_filter(std::is_pointer_v<T>&& std::is_base_of_v<Engine::Object, std::remove_pointer_t<T>>);

	private:
		bool m_is_composite = false;

	public:
		using Property::Property;

		size_t size() const override;
		bool serialize(void* object, Archive& ar) override;

		Engine::Object* object(void* context);
		bool object(void* context, Engine::Object* object);
		const Engine::Object* object(const void* context) const;
		bool is_composite() const;
		ObjectProperty& is_composite(bool flag);

		virtual Class* class_instance() const = 0;
	};

	class ENGINE_EXPORT StructProperty : public Property
	{
		declare_reflect_type(StructProperty, Property);

	private:
		template<typename T>
		using refl_detector = std::enable_if_t<std::is_same_v<decltype(T::static_struct_instance()), Struct*>>;

		trinex_refl_prop_type_filter(std::is_class_v<T>&& is_detected_v<refl_detector, T>);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;
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

		template<typename T>
		static Property* construct_element_property()
		{
			static Property* instance = []() -> Property* {
				using Value                                = typename T::value_type;
				constexpr Value ArrayProperty::* null_prop = nullptr;
				return Object::new_instance<NativeProperty<null_prop>>(nullptr, StringView("Element"));
			}();

			return instance;
		}

		trinex_refl_prop_type_filter(IsArray<T>::value);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;

		virtual const String& index_name(const void* object, size_t index) const;
		virtual Property* element_property() const = 0;
		virtual size_t element_size() const        = 0;

		virtual void* at(void* context, size_t index, bool is_vector_context = false)                   = 0;
		virtual const void* at(const void* context, size_t index, bool is_vector_context = false) const = 0;

		virtual size_t length(const void* context, bool is_vector_context = false) const                             = 0;
		virtual ArrayProperty& emplace_back(void* context, bool is_vector_context = false)                           = 0;
		virtual ArrayProperty& pop_back(void* context, bool is_vector_context = false)                               = 0;
		virtual ArrayProperty& insert(void* context, size_t index, size_t count = 1, bool is_vector_context = false) = 0;
		virtual ArrayProperty& erase(void* context, size_t index, size_t count = 1, bool is_vector_context = false)  = 0;
		virtual ArrayProperty& resize(void* context, size_t new_size, bool is_vector_context = false)                = 0;

		template<typename T>
		T* at_as(void* context, size_t index, bool is_vector_context = false)
		{
			return reinterpret_cast<T*>(at(context, index, is_vector_context));
		}

		template<typename T>
		const T* at_as(const void* context, size_t index, bool is_vector_context = false) const
		{
			return reinterpret_cast<const T*>(at(context, index, is_vector_context));
		}
	};

	class ENGINE_EXPORT ReflObjectProperty : public Property
	{
		declare_reflect_type(ReflObjectProperty, Property);
		trinex_refl_prop_type_filter(std::is_pointer_v<T>&& std::is_base_of_v<Engine::Refl::Object, std::remove_pointer_t<T>>);

	public:
		using Property::Property;

		size_t size() const override;
		bool serialize(void* object, Archive& ar) override;

		Refl::Object* object(void* context);
		bool object(void* context, Refl::Object* object);
		const Engine::Refl::Object* object(const void* context) const;

		virtual Refl::ClassInfo* info() const = 0;
	};

	class ENGINE_EXPORT SubClassProperty : public ReflObjectProperty
	{
		declare_reflect_type(SubClassProperty, ReflObjectProperty);

		template<typename T>
		struct IsSubClassProp {
			static constexpr inline bool value = false;
		};

		template<typename T>
		struct IsSubClassProp<SubClassOf<T>> {
			static constexpr inline bool value = true;
		};

		trinex_refl_prop_type_filter(IsSubClassProp<T>::value);

	public:
		using ReflObjectProperty::ReflObjectProperty;

		Class* class_instance(void* context);
		bool class_instance(void* context, Refl::Class* instance);
		const Class* class_instance(const void* context) const;
		Refl::ClassInfo* info() const override;

		virtual Refl::Class* base_class() const = 0;
	};

	class ENGINE_EXPORT FlagsProperty : public Property
	{
		declare_reflect_type(FlagsProperty, Property);

		template<typename T>
		using enum_detector =
				std::enable_if_t<std::is_enum_v<typename T::Enum> && T::is_enum && T::is_bitfield_enum && T::is_enum_reflected>;
		trinex_refl_prop_type_filter(is_detected_v<enum_detector, T> && sizeof(T) <= sizeof(EnumerateType));

	public:
		virtual Refl::Enum* enum_instance() const = 0;
		bool serialize(void* object, Archive& ar) override;
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

		size_t size() const override { return sizeof(Type); }

		TypedProperty& on_property_changed(const PropertyChangedEvent& event) override
		{
			if constexpr (std::is_base_of_v<Engine::Object, Instance>)
			{
				Property::trigger_object_event(event);
			}
			Super::on_property_changed(event);
			return *this;
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

		bool is_signed() const override { return std::is_signed_v<T>; }
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
		Property* m_inner_property = nullptr;

	public:
		using Super = TypedProperty<prop, VectorProperty>;
		using Super::Super;

		NativePropertyTyped& construct() override
		{
			Super::construct();
			constexpr typename T::value_type T::* inner_prop = nullptr;

			m_inner_property = Object::new_instance<NativeProperty<inner_prop>>(nullptr, "Value");
			return *this;
		}

		size_t length() const override { return T::length(); }
		Property* element_property() const override { return m_inner_property; }
		size_t element_size() const override { return sizeof(typename T::value_type); }

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

		~NativePropertyTyped() override { Refl::Object::destroy_instance(m_inner_property); }
	};

	template<auto prop, typename T>
		requires(MatrixProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, MatrixProperty> {
	private:
		Property* m_inner_property = nullptr;

	public:
		using Super = TypedProperty<prop, MatrixProperty>;
		using Super::Super;

		NativePropertyTyped& construct() override
		{
			Super::construct();
			constexpr typename T::row_type T::* inner_prop = nullptr;
			m_inner_property                               = Object::new_instance<NativeProperty<inner_prop>>(nullptr, "Value");
			return *this;
		}

		size_t columns() const override { return T::col_type::length(); }

		size_t rows() const override { return T::row_type::length(); }

		Property* row_property() const override { return m_inner_property; }

		size_t row_size() const override { return sizeof(typename T::row_type); }

		void* row_address(void* context, size_t index, bool is_vector_context = false) override
		{
			if (index >= static_cast<size_t>(T::row_type::length()))
				return nullptr;

			if (!is_vector_context)
				context = this->address(context);

			return reinterpret_cast<byte*>(context) + sizeof(typename T::row_type) * index;
		}

		const void* row_address(const void* context, size_t index, bool is_vector_context = false) const override
		{
			if (index >= static_cast<size_t>(T::row_type::length()))
				return nullptr;

			if (!is_vector_context)
				context = this->address(context);

			return reinterpret_cast<const byte*>(context) + sizeof(typename T::row_type) * index;
		}

		~NativePropertyTyped() override { Refl::Object::destroy_instance(m_inner_property); }
	};

	template<auto prop, typename T>
		requires(EnumProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, EnumProperty> {
		using Super = TypedProperty<prop, EnumProperty>;
		using Super::Super;

		Enum* enum_instance() const override { return T::static_enum_instance(); }
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

		Class* class_instance() const override { return std::remove_pointer_t<T>::static_class_instance(); }
	};

	template<auto prop, typename T>
		requires(StructProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, StructProperty> {
		using Super = TypedProperty<prop, StructProperty>;
		using Super::Super;

		Struct* struct_instance() const override { return T::static_struct_instance(); }
	};

	template<auto prop, typename T>
		requires(ArrayProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, ArrayProperty> {
		using Super = TypedProperty<prop, ArrayProperty>;
		using Super::Super;
		using Value = typename T::value_type;

	public:
		Property* element_property() const override { return ArrayProperty::construct_element_property<T>(); }
		size_t element_size() const override { return sizeof(Value); }

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

		void* at(void* context, size_t index, bool is_vector_context = false) override
		{
			T& array = array_of(context, is_vector_context);

			if (array.size() <= index)
				return nullptr;

			return array.data() + index;
		}

		const void* at(const void* context, size_t index, bool is_vector_context = false) const override
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

	template<auto prop, typename T>
		requires(ReflObjectProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, ReflObjectProperty> {
		using Super = TypedProperty<prop, ReflObjectProperty>;
		using Super::Super;

		Refl::ClassInfo* info() const override { return std::remove_pointer_t<T>::static_refl_class_info(); }
	};

	template<auto prop, typename T>
		requires(SubClassProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, SubClassProperty> {
		using Super = TypedProperty<prop, SubClassProperty>;
		using Super::Super;

		Class* base_class() const override { return T::Type::static_class_instance(); }
	};

	template<auto prop, typename T>
		requires(FlagsProperty::is_supported<T>)
	struct NativePropertyTyped<prop, T> : public TypedProperty<prop, FlagsProperty> {
		using Super = TypedProperty<prop, FlagsProperty>;
		using Super::Super;

		Enum* enum_instance() const override { return T::static_enum_instance(); }
	};


#undef trinex_refl_prop_type_filter
#define trinex_refl_prop(self, class_name, prop_name, ...)                                                                       \
	self->new_child<Engine::Refl::NativeProperty<&class_name::prop_name>>(#prop_name __VA_OPT__(, ) __VA_ARGS__)

#define trinex_refl_prop_ext(extension, self, class_name, prop_name, ...)                                                        \
	self->new_child<extension<Engine::Refl::NativeProperty<&class_name::prop_name>>>(#prop_name __VA_OPT__(, ) __VA_ARGS__)
}// namespace Engine::Refl
