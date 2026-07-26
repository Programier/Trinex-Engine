#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/etl/function.hpp>
#include <Core/types/name.hpp>
#include <UI/types.hpp>

namespace Trinex::UI
{
	class Document;
}

namespace Trinex::UI::Refl
{
	class Type;

	template<typename T>
	class NativeType;

	struct PropertyRef {
		void* address     = nullptr;
		const Type* type  = nullptr;
		const Name* field = nullptr;
		u64 fields        = 0;
	};

	struct ConstPropertyRef {
		const void* address = nullptr;
		const Type* type    = nullptr;
		const Name* field   = nullptr;
		u64 fields          = 0;
	};

	struct ValueRef {
		void* address    = nullptr;
		const Type* type = nullptr;
	};

	struct ConstValueRef {
		const void* address = nullptr;
		const Type* type    = nullptr;
	};

	class Property
	{
	public:
		struct Flags {
			enum Enum : u8
			{
				Undefined = 0,
				Markup    = 1 << 0,
				Style     = 1 << 1,
			};

			trinex_bitfield_enum_struct(Flags, u8);
		};

		using enum Flags::Enum;

	private:
		Type* m_owner;
		Flags m_flags;

	protected:
	public:
		inline Property(Type* owner = nullptr, Flags flags = Flags::Markup) : m_owner(owner), m_flags(flags) {}

		virtual bool store(void* object, const FunctionRef<bool(void*, Type*)>& writer)            = 0;
		virtual bool load(const void* object, const FunctionRef<bool(const void*, Type*)>& loader) = 0;

		virtual Type* type() const = 0;
		inline Type* owner() const { return m_owner; }
		inline Flags flags() const { return m_flags; }
		virtual ~Property() {}
	};

	template<typename Field, typename Instance>
	class MemberProperty final : public Property
	{
	private:
		Field Instance::* m_property;

	public:
		MemberProperty(Field Instance::* property, Type* owner = nullptr, Flags flags = Flags::Markup)
		    : Property(owner, flags), m_property(property)
		{}

		Type* type() const override { return NativeType<Field>::instance(); }

		bool store(void* object, const FunctionRef<bool(void*, Type*)>& writer) override
		{
			Field* field = &(static_cast<Instance*>(object)->*m_property);
			return writer(field, NativeType<Field>::instance());
		}

		bool load(const void* object, const FunctionRef<bool(const void*, Type*)>& loader) override
		{
			const Field* field = &(static_cast<const Instance*>(object)->*m_property);
			return loader(field, NativeType<Field>::instance());
		}
	};

	template<typename Field>
	class StaticProperty final : public Property
	{
	private:
		Field* m_property;

	public:
		StaticProperty(Field* property, Type* owner = nullptr, Flags flags = Flags::Markup)
		    : Property(owner, flags), m_property(property)
		{}

		Type* type() const override { return NativeType<Field>::instance(); }

		bool store(void* object, const FunctionRef<bool(void*, Type*)>& writer) override
		{
			return writer(m_property, NativeType<Field>::instance());
		}

		bool load(const void* object, const FunctionRef<bool(const void*, Type*)>& loader) override
		{
			return loader(m_property, NativeType<Field>::instance());
		}
	};

	template<typename Field, typename Instance, typename Getter = Field (Instance::*)() const>
	class MethodProperty final : public Property
	{
	public:
		using Setter = bool (Instance::*)(Field);

	private:
		Getter m_getter;
		Setter m_setter;

	public:
		MethodProperty(Getter getter, Setter setter, Type* owner = nullptr, Flags flags = Flags::Markup)
		    : Property(owner, flags), m_getter(getter), m_setter(setter)
		{}

		Type* type() const override { return NativeType<Field>::instance(); }

		bool store(void* object, const FunctionRef<bool(void*, Type*)>& writer) override
		{
			Field field;

			if (writer(&field, NativeType<Field>::instance()))
			{
				return (static_cast<Instance*>(object)->*m_setter)(field);
			}

			return false;
		}

		bool load(const void* object, const FunctionRef<bool(const void*, Type*)>& loader) override
		{
			Field field = (static_cast<const Instance*>(object)->*m_getter)();
			return loader(&field, NativeType<Field>::instance());
		}
	};

	template<typename Field>
	class FunctionProperty final : public Property
	{
	public:
		using Getter = Field (*)();
		using Setter = bool (*)(Field);

	private:
		Getter m_getter;
		Setter m_setter;

	public:
		FunctionProperty(Getter getter, Setter setter, Type* owner = nullptr, Flags flags = Flags::Markup)
		    : Property(owner, flags), m_getter(getter), m_setter(setter)
		{}

		Type* type() const override { return NativeType<Field>::instance(); }

		bool store(void* object, const FunctionRef<bool(void*, Type*)>& writer) override
		{
			Field field;

			if (writer(&field, NativeType<Field>::instance()))
			{
				return m_setter(field);
			}

			return false;
		}

		bool load(const void* object, const FunctionRef<bool(const void*, Type*)>& loader) override
		{
			Field field = m_getter();
			return loader(&field, NativeType<Field>::instance());
		}
	};

	class Type
	{
	public:
		using Resolver = bool (*)(void* dst, const void* src, Property::Flags mask);

	private:
		Type* m_parent;
		Vector<Type*> m_childs;
		FlatMap<Name, Property*> m_properties;
		FlatMap<const Type*, Resolver> m_resolvers;

	protected:
		Type(Type* parent);
		virtual ~Type();

		Type& bind(Name name, Property* prop);
		bool binding_path_resolver(void* dst, const void* src, Property::Flags mask);

	public:
		virtual void* factory() const       = 0;
		virtual Type& destroy(void* object) = 0;
		virtual Name name() const           = 0;

		static bool assign(const ValueRef& dst, const ConstValueRef& src, Property::Flags mask);
		static bool assign(const ValueRef& dst, const ConstPropertyRef& src, Property::Flags mask);
		static bool assign(const PropertyRef& dst, const ConstValueRef& src, Property::Flags mask);
		static bool assign(const PropertyRef& dst, const ConstPropertyRef& src, Property::Flags mask);

		virtual bool assign(void* dst, const void* src) const = 0;

		Property* property(Name name) const;
		inline Type* parent() const { return m_parent; }

		Type& bind(Type* type, Resolver resolver);

		template<typename Source>
		Type& bind(Resolver resolver)
		{
			return Type::bind(NativeType<Source>::instance(), resolver);
		}

		template<typename T>
		inline static Name name_of = Name::undefined;

		friend Document;
	};

	template<typename T>
	class NativeType : public Type
	{
	private:
		static Type* resolve_parent()
		{
			if constexpr (requires { typename T::Super; })
			{
				using Super = typename T::Super;
				if constexpr (requires { Super::reflection(); })
				{
					return Super::reflection();
				}
				else
				{
					return NativeType<Super>::instance();
				}
			}
			else
			{
				return nullptr;
			}
		}

	private:
		NativeType(Type* parent = nullptr) : Type(parent)
		{
			Type* binding_path = etl::is_same_v<T, Markup::BindingPath>
			                             ? static_cast<Type*>(this)
			                             : static_cast<Type*>(NativeType<Markup::BindingPath>::instance());

			Type::bind(binding_path, [](void* dst, const void* src, Property::Flags mask) -> bool {
				return NativeType<T>::instance()->binding_path_resolver(dst, src, mask);
			});
		}

	public:
		static NativeType* instance()
		{
			static NativeType type(resolve_parent());
			return &type;
		}

		void* factory() const override
		{
			if constexpr (etl::is_void_v<T>)
			{
				return nullptr;
			}
			else
			{
				return trx_new T();
			}
		}

		NativeType& destroy(void* object) override
		{
			if constexpr (etl::is_void_v<T>)
			{
				return *this;
			}
			else
			{
				trx_delete static_cast<T*>(object);
				return *this;
			}
		}

		Name name() const override { return Type::name_of<T>; }

		bool assign(void* dst, const void* src) const override
		{
			if constexpr (etl::is_void_v<T>)
			{
				return false;
			}
			else
			{
				*static_cast<T*>(dst) = *static_cast<const T*>(src);
				return true;
			}
		}

		using Type::assign;
		using Type::bind;

		template<typename Field, typename Instance = T>
		    requires(etl::is_class_v<Instance> && !etl::is_function_v<Field>)
		NativeType& bind(Name name, Field Instance::* field, typename Property::Flags flags = Property::Markup)
		{
			Type::bind(name, trx_new MemberProperty(field, this, flags));
			return *this;
		}

		template<typename Field>
		    requires(!etl::is_function_v<Field>)
		NativeType& bind(Name name, Field* field, typename Property::Flags flags = Property::Markup)
		{
			Type::bind(name, trx_new StaticProperty(field, this, flags));
			return *this;
		}

		template<typename Value, typename Instance>
		    requires(etl::is_class_v<Instance>)
		NativeType& bind(Name name, Value (Instance::*getter)() const, bool (Instance::*setter)(Value),
		                 Property::Flags flags = Property::Markup)
		{
			Type::bind(name, trx_new MethodProperty(getter, setter, this, flags));
			return *this;
		}

		template<typename Value, typename Instance>
		    requires(etl::is_class_v<Instance>)
		NativeType& bind(Name name, Value (Instance::*getter)(), bool (Instance::*setter)(Value),
		                 Property::Flags flags = Property::Markup)
		{
			Type::bind(name, trx_new MethodProperty(getter, setter, this, flags));
			return *this;
		}

		template<typename Value>
		NativeType& bind(Name name, Value (*getter)(), bool (*setter)(Value), typename Property::Flags flags = Property::Markup)
		{
			Type::bind(name, trx_new FunctionProperty(getter, setter, this, flags));
			return *this;
		}

		friend Document;
	};

	class ElementRegistry final
	{
	public:
		using Container = FlatMap<Name, Type*>;

	private:
		Container m_types;

	public:
		static ElementRegistry* instance();

		template<typename T>
		NativeType<T>* bind(Name name)
		{
			if (m_types.contains(name))
				return nullptr;

			NativeType<T>* element = NativeType<T>::instance();
			Type::name_of<T>       = name;
			m_types.insert({name, element});
			return element;
		}

		inline ElementRegistry& unbind(Name name) { trinex_this_return(m_types.erase(name)); }

		inline Type* find(Name name) const
		{
			auto it = m_types.find(name);
			return it == m_types.end() ? nullptr : it->second;
		}

		inline const Container& types() const { return m_types; }
	};

#define trinex_ui_bind_type_name(name) Trinex::UI::Refl::Type::name_of<name> = #name
}// namespace Trinex::UI::Refl
