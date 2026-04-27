#pragma once
#include <Core/etl/any.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/math/angle.hpp>
#include <Core/reflection/object.hpp>
#include <Core/types/color.hpp>

namespace Trinex
{
	class Object;
	class ScriptFunction;
	class Path;
}// namespace Trinex

namespace Trinex::Refl
{
	template<typename T>
	class SubClassOf;

	class ArrayProperty;
	class PropertyRenderer;

	struct PropertyChangedEvent {
		void* context;
		Property* property;
		PropertyChangedEvent* owner_event;
		PropertyChangedEvent* member_event;

		PropertyChangedEvent(void* context, Property* property, PropertyChangedEvent* owner_event = nullptr,
		                     PropertyChangedEvent* member_event = nullptr)
		    : context(context), property(property), owner_event(owner_event), member_event(member_event)
		{}

		template<typename T>
		T* context_as() const
		{
			return reinterpret_cast<T*>(context);
		}

		inline bool is_a(const void* field) const;
	};


#define trinex_refl_prop_type_filter(...)                                                                                        \
public:                                                                                                                          \
	template<typename T>                                                                                                         \
	static constexpr inline bool is_supported = !std::is_const_v<std::remove_pointer_t<T>> && __VA_ARGS__;                       \
                                                                                                                                 \
private:
	namespace Detail
	{
		template<typename T>
		struct ContextAccessor;

		template<typename Getter, typename Enable>
		struct GetterBinding;

		template<typename Accessor>
		struct PropertyImpl;
	}// namespace Detail

	class ENGINE_EXPORT Property : public Object
	{
		trinex_reflect_type(Property, Object);

	public:
		enum Flag
		{
			IsReadOnly        = BIT(0),
			IsTransient       = BIT(1),
			IsHidden          = BIT(2),
			InlineSingleField = BIT(3),
			Inline            = BIT(4),
		};

		using ChangeListener = Function<void(const PropertyChangedEvent&)>;
		using RenderFunction = Function<bool(PropertyRenderer&, Property&, void*)>;

		template<typename T>
		static Property* null_property()
		{
			using PropType = Detail::PropertyImpl<Detail::ContextAccessor<T>>;
			static PropType* property =
			        Object::new_instance<PropType>(nullptr, StringView("Element"), Detail::ContextAccessor<T>(), 0);
			return property;
		}

		template<typename T>
		static Property* create_element(BitMask flags = 0);

		template<typename T, typename Instance>
		static auto create(StringView name, T Instance::* prop, Object* owner = nullptr, BitMask flags = 0)
		    requires(!std::is_function_v<T>);

		template<typename T>
		static auto create(StringView name, T* prop, Object* owner = nullptr, BitMask flags = 0)
		    requires(!std::is_function_v<T>);

		template<typename Ret, typename Instance>
		static auto create(StringView name, Ret (Instance::*getter)() const, Object* owner = nullptr, BitMask flags = IsReadOnly);

		template<typename Ret, typename Instance, typename SetterRet, typename SetterArg>
		static auto create(StringView name, Ret (Instance::*getter)() const, SetterRet (Instance::*setter)(SetterArg),
		                   Object* owner = nullptr, BitMask flags = 0);

		template<typename Ret>
		static auto create(StringView name, Ret (*getter)(), Object* owner = nullptr, BitMask flags = IsReadOnly);

		template<typename Ret, typename SetterRet, typename SetterArg>
		static auto create(StringView name, Ret (*getter)(), SetterRet (*setter)(SetterArg), Object* owner = nullptr,
		                   BitMask flags = 0);

		template<typename Getter, typename Setter>
		static auto create(StringView name, Getter getter, Setter setter, Object* owner = nullptr, BitMask flags = 0)
		    requires(Detail::GetterBinding<std::decay_t<Getter>, void>::is_supported);

		template<typename Getter>
		static auto create(StringView name, Getter getter, Object* owner = nullptr, BitMask flags = IsReadOnly)
		    requires(Detail::GetterBinding<std::decay_t<Getter>, void>::is_supported);

	protected:
		BitMask m_flags = 0;
		RenderFunction m_render_function;

		template<usize size>
		struct InnerProperties {
			Property* properties[size];
		};

		static void trigger_object_event(const PropertyChangedEvent& event);

		inline bool check_flag(BitMask mask) const { return (m_flags & mask) == mask; }

	public:
		Property(BitMask flags = 0);

		inline BitMask flags() const { return m_flags; }
		inline bool is_read_only() const { return check_flag(IsReadOnly); }
		inline bool is_transient() const { return check_flag(IsTransient); }
		inline bool is_hidden() const { return check_flag(IsHidden); }
		inline bool is_inline() const { return check_flag(Inline); }
		inline bool is_inline_single_field() const { return check_flag(InlineSingleField); }

		using Super::display_name;
		virtual void* address(void* context)                   = 0;
		virtual const void* address(const void* context) const = 0;
		virtual usize size() const                             = 0;
		virtual usize alignment() const                        = 0;
		virtual bool serialize(void* object, Archive& ar)      = 0;
		virtual bool render(PropertyRenderer& renderer, void* context);
		virtual const String& property_name(const void* context);
		virtual Property& on_property_changed(const PropertyChangedEvent& event);
		virtual Property& item_flags(BitMask flags);
		Property& render_function(RenderFunction function);

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

	inline bool PropertyChangedEvent::is_a(const void* field) const
	{
		return property->address(context) == field;
	}

	struct PropertyLayout
	{
		bool draw_label      = true;
		bool inline_value    = false;
		bool inline_children = false;
		bool single_field    = false;
	};

	class ENGINE_EXPORT PropertyRenderer
	{
	public:
		virtual String label(Property& property, const void* context)                              = 0;
		virtual String item_label(ArrayProperty& property, const void* context, usize index)       = 0;
		virtual PropertyLayout layout(Property& property, const void* context)                      = 0;
		virtual bool begin_property(Property& property, void* context, StringView label,
		                            const PropertyLayout& layout)                                   = 0;
		virtual void end_property(Property& property, void* context, const PropertyLayout& layout) = 0;
		virtual bool begin_children(Property& property, void* context, const PropertyLayout& layout) = 0;
		virtual void end_children(Property& property, void* context, const PropertyLayout& layout) = 0;
		virtual bool render_default(Property& property, void* context)                              = 0;
		virtual bool render_children(Property& property, void* context)                             = 0;
		virtual bool edit_value(StringView label, Any& value)                                       = 0;
		virtual void property_changed(Property& property, void* context)                            = 0;
		virtual ~PropertyRenderer()                                                                 = default;
	};

	class ENGINE_EXPORT RedirectorProperty : public Property
	{
		trinex_reflect_type(RedirectorProperty, Property);

	private:
		Property* m_property;
		usize m_offset;

	public:
		RedirectorProperty(Property* property, usize offset);

		void* address(void* context) override;
		const void* address(const void* context) const override;
		usize size() const override;
		usize alignment() const override;
		bool serialize(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT PrimitiveProperty : public Property
	{
		trinex_reflect_type(PrimitiveProperty, Property);

	public:
		using Property::Property;
		bool serialize(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT BooleanProperty : public PrimitiveProperty
	{
		trinex_reflect_type(BooleanProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(std::is_same_v<T, bool>);

	public:
		using PrimitiveProperty::PrimitiveProperty;
		usize size() const override;
		usize alignment() const override;
	};

	class ENGINE_EXPORT IntegerProperty : public PrimitiveProperty
	{
		trinex_reflect_type(IntegerProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(!std::is_same_v<T, bool> && std::is_integral_v<T> && !std::is_enum_v<T>);

	public:
		using PrimitiveProperty::PrimitiveProperty;
		bool is_unsigned() const;
		virtual bool is_signed() const = 0;
	};

	class ENGINE_EXPORT FloatProperty : public PrimitiveProperty
	{
		trinex_reflect_type(FloatProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(std::is_floating_point_v<T>);

	public:
		using PrimitiveProperty::PrimitiveProperty;
	};

	class ENGINE_EXPORT AngleProperty : public PrimitiveProperty
	{
		trinex_reflect_type(AngleProperty, PrimitiveProperty);
		trinex_refl_prop_type_filter(std::is_same_v<T, Angle>);

	public:
		using PrimitiveProperty::PrimitiveProperty;
	};

	class ENGINE_EXPORT VectorProperty : public PrimitiveProperty
	{
		trinex_reflect_type(VectorProperty, PrimitiveProperty);

	private:
		template<typename T>
		struct IsVector : std::false_type {
		};

		template<glm::length_t L, typename T, glm::qualifier Q>
		struct IsVector<glm::vec<L, T, Q>> : std::true_type {
		};

		trinex_refl_prop_type_filter(IsVector<T>::value);

	protected:
		BitMask m_element_property_flags = 0;

	public:
		using PrimitiveProperty::PrimitiveProperty;

		VectorProperty& element_flags(BitMask flags);
		virtual usize length() const               = 0;
		virtual Property* element_property() const = 0;
		virtual usize element_size() const         = 0;

		virtual void* element_address(void* context, usize index, bool is_matrix_context = false)                   = 0;
		virtual const void* element_address(const void* context, usize index, bool is_matrix_context = false) const = 0;

		template<typename T>
		T* element_address_as(void* context, usize index, bool is_matrix_context = false)
		{
			return reinterpret_cast<T*>(element_address(context, index, is_matrix_context));
		}

		template<typename T>
		const T* element_address_as(const void* context, usize index, bool is_vector_context = false) const
		{
			return reinterpret_cast<const T*>(element_address(context, index, is_vector_context));
		}
	};

	class ENGINE_EXPORT MatrixProperty : public PrimitiveProperty
	{
		trinex_reflect_type(MatrixProperty, PrimitiveProperty);

	private:
		template<typename T>
		struct IsMatrix : std::false_type {
		};

		template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
		struct IsMatrix<glm::mat<C, R, T, Q>> : std::true_type {
		};

		trinex_refl_prop_type_filter(IsMatrix<T>::value);

	protected:
		BitMask m_row_property_flags = 0;

	public:
		using PrimitiveProperty::PrimitiveProperty;

		MatrixProperty& row_flags(BitMask flags);
		virtual usize columns() const          = 0;
		virtual usize rows() const             = 0;
		virtual Property* row_property() const = 0;
		virtual usize row_size() const         = 0;

		virtual void* row_address(void* context, usize index, bool is_vector_context = false)                   = 0;
		virtual const void* row_address(const void* context, usize index, bool is_vector_context = false) const = 0;

		template<typename T>
		T* row_address_as(void* context, usize index, bool is_vector_context = false)
		{
			return reinterpret_cast<T*>(row_address(context, index, is_vector_context));
		}

		template<typename T>
		const T* row_address_as(const void* context, usize index, bool is_vector_context = false) const
		{
			return reinterpret_cast<const T*>(row_address(context, index, is_vector_context));
		}
	};

	class ENGINE_EXPORT QuaternionProperty : public PrimitiveProperty
	{
		trinex_reflect_type(QuaternionProperty, PrimitiveProperty);

	private:
		trinex_refl_prop_type_filter(std::is_same_v<T, Quaternion>);

	public:
		using PrimitiveProperty::PrimitiveProperty;
	};

	class ENGINE_EXPORT EnumProperty : public PrimitiveProperty
	{
		trinex_reflect_type(EnumProperty, PrimitiveProperty);

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

	class ENGINE_EXPORT ColorProperty : public PrimitiveProperty
	{
		trinex_refl_prop_type_filter(std::is_same_v<T, Trinex::Color>);
		trinex_reflect_type(ColorProperty, PrimitiveProperty);

	public:
		using PrimitiveProperty::PrimitiveProperty;
	};

	class ENGINE_EXPORT LinearColorProperty : public PrimitiveProperty
	{
		trinex_refl_prop_type_filter(std::is_same_v<T, Trinex::LinearColor>);
		trinex_reflect_type(LinearColorProperty, PrimitiveProperty);

	public:
		using PrimitiveProperty::PrimitiveProperty;
	};

	class ENGINE_EXPORT StringProperty : public Property
	{
		trinex_reflect_type(StringProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<String, T>);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;
		usize size() const override;
		usize alignment() const override;
	};

	class ENGINE_EXPORT NameProperty : public Property
	{
		trinex_reflect_type(NameProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<Name, T>);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT PathProperty : public Property
	{
		trinex_reflect_type(PathProperty, Property);
		trinex_refl_prop_type_filter(std::is_same_v<Path, T>);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT ObjectProperty : public Property
	{
		trinex_reflect_type(ObjectProperty, Property);
		trinex_refl_prop_type_filter(std::is_pointer_v<T>&& std::is_base_of_v<Trinex::Object, std::remove_pointer_t<T>>);

	private:
		bool m_is_composite = false;

	public:
		using Property::Property;

		usize size() const override;
		usize alignment() const override;
		bool serialize(void* object, Archive& ar) override;

		Trinex::Object* object(void* context);
		bool object(void* context, Trinex::Object* object);
		const Trinex::Object* object(const void* context) const;
		bool is_composite() const;
		ObjectProperty& is_composite(bool flag);

		virtual Class* class_instance() const = 0;
	};

	class ENGINE_EXPORT StructProperty : public Property
	{
		trinex_reflect_type(StructProperty, Property);

	private:
		template<typename T>
		using refl_detector = std::enable_if_t<std::is_same_v<decltype(T::static_reflection()), Struct*>>;

		trinex_refl_prop_type_filter(std::is_class_v<T>&& is_detected_v<refl_detector, T>);

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;
		virtual Struct* struct_instance() const = 0;
	};

	class ENGINE_EXPORT ArrayProperty : public Property
	{
		trinex_reflect_type(ArrayProperty, Property);

	protected:
		template<typename T>
		struct IsArray : std::false_type {
		};

		template<typename T, typename Alloc>
		struct IsArray<Trinex::Vector<T, Alloc>> : std::true_type {
		};

		template<typename T>
		struct IsScriptableArray : std::false_type {
		};

		template<typename T>
		struct IsScriptableArray<Trinex::Vector<T>> : std::true_type {
		};

		trinex_refl_prop_type_filter(IsArray<T>::value);

	protected:
		BitMask m_element_property_flags = 0;

	public:
		using Property::Property;

		bool serialize(void* object, Archive& ar) override;

		ArrayProperty& element_flags(BitMask flags);
		virtual const String& index_name(const void* object, usize index) const;
		virtual Property* element_property() const = 0;
		virtual usize element_size() const         = 0;

		virtual void* at(void* context, usize index, bool is_vector_context = false)                   = 0;
		virtual const void* at(const void* context, usize index, bool is_vector_context = false) const = 0;

		virtual usize length(const void* context, bool is_vector_context = false) const                            = 0;
		virtual ArrayProperty& emplace_back(void* context, bool is_vector_context = false)                         = 0;
		virtual ArrayProperty& pop_back(void* context, bool is_vector_context = false)                             = 0;
		virtual ArrayProperty& insert(void* context, usize index, usize count = 1, bool is_vector_context = false) = 0;
		virtual ArrayProperty& erase(void* context, usize index, usize count = 1, bool is_vector_context = false)  = 0;
		virtual ArrayProperty& resize(void* context, usize new_size, bool is_vector_context = false)               = 0;

		template<typename T>
		T* at_as(void* context, usize index, bool is_vector_context = false)
		{
			return reinterpret_cast<T*>(at(context, index, is_vector_context));
		}

		template<typename T>
		const T* at_as(const void* context, usize index, bool is_vector_context = false) const
		{
			return reinterpret_cast<const T*>(at(context, index, is_vector_context));
		}
	};

	class ENGINE_EXPORT ReflObjectProperty : public Property
	{
		trinex_reflect_type(ReflObjectProperty, Property);
		trinex_refl_prop_type_filter(std::is_pointer_v<T>&& std::is_base_of_v<Trinex::Refl::Object, std::remove_pointer_t<T>>);

	public:
		using Property::Property;

		usize size() const override;
		usize alignment() const override;
		bool serialize(void* object, Archive& ar) override;

		Refl::Object* object(void* context);
		bool object(void* context, Refl::Object* object);
		const Trinex::Refl::Object* object(const void* context) const;

		virtual Refl::ClassInfo* info() const = 0;
	};

	class ENGINE_EXPORT SubClassProperty : public ReflObjectProperty
	{
		trinex_reflect_type(SubClassProperty, ReflObjectProperty);

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
		trinex_reflect_type(FlagsProperty, Property);

		template<typename T>
		using enum_detector =
		        std::enable_if_t<std::is_enum_v<typename T::Enum> && T::is_enum && T::is_bitfield_enum && T::is_enum_reflected>;
		trinex_refl_prop_type_filter(is_detected_v<enum_detector, T> && sizeof(T) <= sizeof(EnumerateType));

	public:
		using Property::Property;
		virtual Refl::Enum* enum_instance() const = 0;
		bool serialize(void* object, Archive& ar) override;
	};

	class ENGINE_EXPORT VirtualProperty : public Property
	{
		trinex_reflect_type(VirtualProperty, Property);

	public:
		using Property::Property;

		void* address(void* context) final override;
		const void* address(const void* context) const final override;
		bool serialize(void* object, Archive& ar) override;

		virtual Property* property()                                     = 0;
		virtual Any getter(const void* context)                          = 0;
		virtual VirtualProperty& setter(void* context, const Any& value) = 0;
		virtual Any construct_value() const                              = 0;
	};

	namespace Detail
	{
		template<typename T>
		inline constexpr bool always_false = false;

		template<typename T>
		struct ContextAccessor {
			using Type      = T;
			using OwnerType = void;

			void* address(void* context) const { return context; }
			const void* address(const void* context) const { return context; }

			static constexpr inline bool should_trigger_owner_event = false;
		};

		template<typename T, typename Owner>
		struct MemberAccessor {
			using Type      = T;
			using OwnerType = Owner;

			T Owner::* prop;

			void* address(void* context) const
			{
				Owner* instance = reinterpret_cast<Owner*>(context);
				return &(instance->*prop);
			}

			const void* address(const void* context) const
			{
				const Owner* instance = reinterpret_cast<const Owner*>(context);
				return &(instance->*prop);
			}

			static constexpr inline bool should_trigger_owner_event = std::is_base_of_v<Trinex::Object, Owner>;
		};

		template<typename T>
		struct StaticAccessor {
			using Type      = T;
			using OwnerType = void;

			T* prop;

			void* address(void*) const { return prop; }
			const void* address(const void*) const { return prop; }

			static constexpr inline bool should_trigger_owner_event = false;
		};

		template<typename Accessor, typename Super, typename = std::enable_if_t<std::is_base_of_v<Property, Super>>>
		class PropertyStorage : public Super
		{
		public:
			using Type = typename Accessor::Type;

			Accessor m_accessor;

			PropertyStorage(Accessor accessor, BitMask flags = 0) : Super(flags), m_accessor(std::move(accessor)) {}

			void* address(void* context) override { return m_accessor.address(context); }
			const void* address(const void* context) const override { return m_accessor.address(context); }

			usize size() const override { return sizeof(Type); }
			usize alignment() const override { return alignof(Type); }

			PropertyStorage& on_property_changed(const PropertyChangedEvent& event) override
			{
				if constexpr (Accessor::should_trigger_owner_event)
				{
					Property::trigger_object_event(event);
				}
				Super::on_property_changed(event);
				return *this;
			}
		};

		// Maps a concrete C++ value type to the matching reflected Property subclass
		// while reusing a single accessor-based storage implementation.
		template<typename Accessor, typename T>
		struct PropertyImplTyped {
		};

		template<typename Accessor>
		struct PropertyImpl : public PropertyImplTyped<Accessor, typename Accessor::Type> {
			using Super = PropertyImplTyped<Accessor, typename Accessor::Type>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(BooleanProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, BooleanProperty> {
			using Super = PropertyStorage<Accessor, BooleanProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(IntegerProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, IntegerProperty> {
			using Super = PropertyStorage<Accessor, IntegerProperty>;
			using Super::Super;

			bool is_signed() const override { return std::is_signed_v<T>; }
		};

		template<typename Accessor, typename T>
		    requires(FloatProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, FloatProperty> {
			using Super = PropertyStorage<Accessor, FloatProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(AngleProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, AngleProperty> {
			using Super = PropertyStorage<Accessor, AngleProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(VectorProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, VectorProperty> {
			using Super = PropertyStorage<Accessor, VectorProperty>;
			using Super::Super;

			usize length() const override { return T::length(); }
			Property* element_property() const override
			{
				return Property::create_element<typename T::value_type>(this->m_element_property_flags);
			}
			usize element_size() const override { return sizeof(typename T::value_type); }

			void* element_address(void* context, usize index, bool is_vector_context = false) override
			{
				if (index >= static_cast<usize>(T::length()))
					return nullptr;

				if (!is_vector_context)
					context = this->address(context);

				return reinterpret_cast<u8*>(context) + sizeof(typename T::value_type) * index;
			}

			const void* element_address(const void* context, usize index, bool is_vector_context = false) const override
			{
				if (index >= static_cast<usize>(T::length()))
					return nullptr;

				if (!is_vector_context)
					context = this->address(context);

				return reinterpret_cast<const u8*>(context) + sizeof(typename T::value_type) * index;
			}
		};

		template<typename Accessor, typename T>
		    requires(MatrixProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, MatrixProperty> {
			using Super = PropertyStorage<Accessor, MatrixProperty>;
			using Super::Super;

			usize columns() const override { return T::col_type::length(); }
			usize rows() const override { return T::row_type::length(); }
			Property* row_property() const override
			{
				return Property::create_element<typename T::row_type>(this->m_row_property_flags);
			}
			usize row_size() const override { return sizeof(typename T::row_type); }

			void* row_address(void* context, usize index, bool is_vector_context = false) override
			{
				if (index >= static_cast<usize>(T::row_type::length()))
					return nullptr;

				if (!is_vector_context)
					context = this->address(context);

				return reinterpret_cast<u8*>(context) + sizeof(typename T::row_type) * index;
			}

			const void* row_address(const void* context, usize index, bool is_vector_context = false) const override
			{
				if (index >= static_cast<usize>(T::row_type::length()))
					return nullptr;

				if (!is_vector_context)
					context = this->address(context);

				return reinterpret_cast<const u8*>(context) + sizeof(typename T::row_type) * index;
			}
		};

		template<typename Accessor, typename T>
		    requires(QuaternionProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, QuaternionProperty> {
			using Super = PropertyStorage<Accessor, QuaternionProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(EnumProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, EnumProperty> {
			using Super = PropertyStorage<Accessor, EnumProperty>;
			using Super::Super;

			Enum* enum_instance() const override { return T::static_reflection(); }
		};

		template<typename Accessor, typename T>
		    requires(ColorProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, ColorProperty> {
			using Super = PropertyStorage<Accessor, ColorProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(LinearColorProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, LinearColorProperty> {
			using Super = PropertyStorage<Accessor, LinearColorProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(StringProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, StringProperty> {
			using Super = PropertyStorage<Accessor, StringProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(NameProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, NameProperty> {
			using Super = PropertyStorage<Accessor, NameProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(PathProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, PathProperty> {
			using Super = PropertyStorage<Accessor, PathProperty>;
			using Super::Super;
		};

		template<typename Accessor, typename T>
		    requires(ObjectProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, ObjectProperty> {
			using Super = PropertyStorage<Accessor, ObjectProperty>;
			using Super::Super;

			Class* class_instance() const override { return std::remove_pointer_t<T>::static_reflection(); }
		};

		template<typename Accessor, typename T>
		    requires(StructProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, StructProperty> {
			using Super = PropertyStorage<Accessor, StructProperty>;
			using Super::Super;

			Struct* struct_instance() const override { return T::static_reflection(); }
		};

		template<typename Accessor, typename T>
		    requires(ArrayProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, ArrayProperty> {
			using Super = PropertyStorage<Accessor, ArrayProperty>;
			using Super::Super;
			using Value = typename T::value_type;

			Property* element_property() const override { return Property::create_element<Value>(this->m_element_property_flags); }
			usize element_size() const override { return sizeof(Value); }

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

			void* at(void* context, usize index, bool is_vector_context = false) override
			{
				T& array = array_of(context, is_vector_context);

				if (array.size() <= index)
					return nullptr;

				return array.data() + index;
			}

			const void* at(const void* context, usize index, bool is_vector_context = false) const override
			{
				const T& array = array_of(context, is_vector_context);

				if (array.size() <= index)
					return nullptr;

				return array.data() + index;
			}

			usize length(const void* context, bool is_vector_context = false) const override
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

			ArrayProperty& insert(void* context, usize index, usize count = 1, bool is_vector_context = false) override
			{
				T& array = array_of(context, is_vector_context);
				array.insert(array.begin() + index, count, Value());
				return *this;
			}

			ArrayProperty& erase(void* context, usize index, usize count = 1, bool is_vector_context = false) override
			{
				T& array = array_of(context, is_vector_context);
				array.erase(array.begin() + index, array.begin() + index + count);
				return *this;
			}

			ArrayProperty& resize(void* context, usize new_size, bool is_vector_context = false) override
			{
				array_of(context, is_vector_context).resize(new_size, Value());
				return *this;
			}
		};

		template<typename Accessor, typename T>
		    requires(ReflObjectProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, ReflObjectProperty> {
			using Super = PropertyStorage<Accessor, ReflObjectProperty>;
			using Super::Super;

			Refl::ClassInfo* info() const override { return std::remove_pointer_t<T>::static_refl_class_info(); }
		};

		template<typename Accessor, typename T>
		    requires(SubClassProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, SubClassProperty> {
			using Super = PropertyStorage<Accessor, SubClassProperty>;
			using Super::Super;

			Class* base_class() const override { return T::Type::static_reflection(); }
		};

		template<typename Accessor, typename T>
		    requires(FlagsProperty::is_supported<T>)
		struct PropertyImplTyped<Accessor, T> : public PropertyStorage<Accessor, FlagsProperty> {
			using Super = PropertyStorage<Accessor, FlagsProperty>;
			using Super::Super;

			Enum* enum_instance() const override { return T::static_reflection(); }
		};

		using CallbackGetter = Function<Any(const void*)>;
		using CallbackSetter = Function<void(void*, const Any&)>;

		template<typename Getter, typename Enable>
		struct GetterBinding {
			static constexpr inline bool is_supported = false;
		};

		template<typename Ret, typename Instance>
		struct GetterBinding<Ret (Instance::*)() const, void> {
			using Value                               = std::remove_cvref_t<Ret>;
			static constexpr inline bool is_supported = true;

			static CallbackGetter bind(Ret (Instance::*getter)() const)
			{
				return [getter](const void* context) -> Any {
					const Instance* object = static_cast<const Instance*>(context);
					return Any((object->*getter)());
				};
			}
		};

		template<typename Ret>
		struct GetterBinding<Ret (*)(), void> {
			using Value                               = std::remove_cvref_t<Ret>;
			static constexpr inline bool is_supported = true;

			static CallbackGetter bind(Ret (*getter)())
			{
				return [getter](const void*) -> Any { return Any(getter()); };
			}
		};

		template<typename Getter>
		struct GetterBinding<Getter, std::enable_if_t<!std::is_member_function_pointer_v<Getter> &&
		                                              std::is_invocable_v<Getter, const void*>>> {
			using Value                               = std::remove_cvref_t<std::invoke_result_t<Getter, const void*>>;
			static constexpr inline bool is_supported = true;

			static CallbackGetter bind(Getter getter)
			{
				return [getter = std::move(getter)](const void* context) -> Any { return Any(getter(context)); };
			}
		};

		template<typename Setter, typename Value, typename = void>
		struct SetterBinding {
			static constexpr inline bool is_supported = false;
		};

		template<typename Ret, typename Instance, typename Arg, typename Value>
		struct SetterBinding<Ret (Instance::*)(Arg), Value,
		                     std::enable_if_t<std::is_constructible_v<std::remove_cvref_t<Arg>, const Value&>>> {
			static constexpr inline bool is_supported = true;

			static CallbackSetter bind(Ret (Instance::*setter)(Arg))
			{
				return [setter](void* context, const Any& value) {
					Instance* object  = static_cast<Instance*>(context);
					Value typed_value = value.cast<Value>();
					(object->*setter)(static_cast<Arg>(typed_value));
				};
			}
		};

		template<typename Ret, typename Arg, typename Value>
		struct SetterBinding<Ret (*)(Arg), Value,
		                     std::enable_if_t<std::is_constructible_v<std::remove_cvref_t<Arg>, const Value&>>> {
			static constexpr inline bool is_supported = true;

			static CallbackSetter bind(Ret (*setter)(Arg))
			{
				return [setter](void*, const Any& value) {
					Value typed_value = value.cast<Value>();
					setter(static_cast<Arg>(typed_value));
				};
			}
		};

		template<typename Setter, typename Value>
		struct SetterBinding<Setter, Value,
		                     std::enable_if_t<!std::is_member_function_pointer_v<Setter> &&
		                                      std::is_invocable_v<Setter, void*, const Value&>>> {
			static constexpr inline bool is_supported = true;

			static CallbackSetter bind(Setter setter)
			{
				return [setter = std::move(setter)](void* context, const Any& value) {
					Value typed_value = value.cast<Value>();
					setter(context, typed_value);
				};
			}
		};

		class CallbackPropertyBase : public VirtualProperty
		{
		protected:
			CallbackGetter m_getter;
			CallbackSetter m_setter;

			CallbackPropertyBase(CallbackGetter getter, CallbackSetter setter, BitMask flags)
			    : VirtualProperty(flags | (setter ? 0 : Property::IsReadOnly)), m_getter(std::move(getter)),
			      m_setter(std::move(setter))
			{}

			Any getter(const void* context) override { return m_getter(context); }

			VirtualProperty& setter(void* context, const Any& value) override
			{
				if (m_setter)
				{
					m_setter(context, value);
				}
				return *this;
			}
		};

		template<typename T>
		class CallbackProperty : public CallbackPropertyBase
		{
		public:
			CallbackProperty(CallbackGetter getter, CallbackSetter setter, BitMask flags)
			    : CallbackPropertyBase(std::move(getter), std::move(setter), flags)
			{}

			Any construct_value() const override { return T(); }
			Property* property() override { return Property::null_property<T>(); }
			usize size() const override { return sizeof(T); }
			usize alignment() const override { return alignof(T); }
		};

		template<typename Value>
		inline auto create_callback_property(Object* owner, StringView name, CallbackGetter getter, CallbackSetter setter,
		                                     BitMask flags)
		{
			using PropType = CallbackProperty<Value>;
			return Object::new_instance<PropType>(owner, name, std::move(getter), std::move(setter), flags);
		}
	}// namespace Detail

	template<typename T, typename Instance>
	inline auto Property::create(StringView name, T Instance::* prop, Object* owner, BitMask flags)
	    requires(!std::is_function_v<T>)
	{
		using Accessor = Detail::MemberAccessor<T, Instance>;
		using PropType = Detail::PropertyImpl<Accessor>;
		return Object::new_instance<PropType>(owner, name, Accessor{prop}, flags);
	}

	template<typename T>
	inline Property* Property::create_element(BitMask flags)
	{
		using PropType = Detail::PropertyImpl<Detail::ContextAccessor<T>>;

		if (flags == 0)
		{
			return null_property<T>();
		}

		return Object::new_instance<PropType>(nullptr, StringView("Element"), Detail::ContextAccessor<T>(), flags);
	}

	template<typename T>
	inline auto Property::create(StringView name, T* prop, Object* owner, BitMask flags)
	    requires(!std::is_function_v<T>)
	{
		using Accessor = Detail::StaticAccessor<T>;
		using PropType = Detail::PropertyImpl<Accessor>;
		return Object::new_instance<PropType>(owner, name, Accessor{prop}, flags);
	}

	template<typename Ret, typename Instance>
	inline auto Property::create(StringView name, Ret (Instance::*getter)() const, Object* owner, BitMask flags)
	{
		return create<Ret (Instance::*)() const>(name, getter, owner, flags);
	}

	template<typename Ret, typename Instance, typename SetterRet, typename SetterArg>
	inline auto Property::create(StringView name, Ret (Instance::*getter)() const, SetterRet (Instance::*setter)(SetterArg),
	                             Object* owner, BitMask flags)
	{
		return create<Ret (Instance::*)() const, SetterRet (Instance::*)(SetterArg)>(name, getter, setter, owner, flags);
	}

	template<typename Ret>
	inline auto Property::create(StringView name, Ret (*getter)(), Object* owner, BitMask flags)
	{
		return create<Ret (*)()>(name, getter, owner, flags);
	}

	template<typename Ret, typename SetterRet, typename SetterArg>
	inline auto Property::create(StringView name, Ret (*getter)(), SetterRet (*setter)(SetterArg), Object* owner, BitMask flags)
	{
		return create<Ret (*)(), SetterRet (*)(SetterArg)>(name, getter, setter, owner, flags);
	}

	template<typename Getter, typename Setter>
	inline auto Property::create(StringView name, Getter getter, Setter setter, Object* owner, BitMask flags)
	    requires(Detail::GetterBinding<std::decay_t<Getter>, void>::is_supported)
	{
		using GetterBinding = Detail::GetterBinding<std::decay_t<Getter>, void>;
		using Value         = typename GetterBinding::Value;
		using SetterBinding = Detail::SetterBinding<std::decay_t<Setter>, Value>;

		static_assert(SetterBinding::is_supported, "Unsupported property setter signature");

		return Detail::create_callback_property<Value>(owner, name, GetterBinding::bind(std::move(getter)),
		                                               SetterBinding::bind(std::move(setter)), flags);
	}

	template<typename Getter>
	inline auto Property::create(StringView name, Getter getter, Object* owner, BitMask flags)
	    requires(Detail::GetterBinding<std::decay_t<Getter>, void>::is_supported)
	{
		using GetterBinding = Detail::GetterBinding<std::decay_t<Getter>, void>;
		using Value         = typename GetterBinding::Value;
		return Detail::create_callback_property<Value>(owner, name, GetterBinding::bind(std::move(getter)), {},
		                                               flags | IsReadOnly);
	}

#undef trinex_refl_prop_type_filter
#define trinex_refl_prop(prop_name, ...)                                                                                         \
	Trinex::Refl::Property::create(#prop_name, &This::prop_name, This::static_reflection() __VA_OPT__(, ) __VA_ARGS__)

#define trinex_refl_virtual_prop(prop_name, getter, setter, ...)                                                                 \
	Trinex::Refl::Property::create(#prop_name, &This::getter, &This::setter, This::static_reflection() __VA_OPT__(, ) __VA_ARGS__)

}// namespace Trinex::Refl
