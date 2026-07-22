#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/types/name.hpp>
#include <UI/types.hpp>

namespace Trinex::UI::Refl
{
	class Type;

	class Property
	{
	private:
		Type* m_owner;

	protected:
		static bool assign(void* field, Type* type, const Markup::ValueDesc& value);
		static bool assign(bool* field, Type* type, const Markup::ValueDesc& value);
		static bool assign(i32* field, Type* type, const Markup::ValueDesc& value);
		static bool assign(f32* field, Type* type, const Markup::ValueDesc& value);
		static bool assign(String* field, Type* type, const Markup::ValueDesc& value);

	public:
		inline Property(Type* owner = nullptr) : m_owner(owner) {}

		virtual Type* type() const                                              = 0;
		virtual bool assign(void* object, const Markup::ValueDesc& value) const = 0;

		inline Type* owner() const { return m_owner; }
		virtual ~Property() {}
	};

	template<typename Field, typename Instance>
	class MemberProperty final : public Property
	{
	private:
		Field Instance::* m_property;

	public:
		MemberProperty(Field Instance::* property, Type* owner = nullptr) : Property(owner), m_property(property) {}
		Type* type() const override;

		bool assign(void* object, const Markup::ValueDesc& value) const override
		{
			Field* address = &(static_cast<Instance*>(object)->*m_property);
			return Property::assign(address, MemberProperty::type(), value);
		}
	};

	template<typename Field>
	class StaticProperty final : public Property
	{
	private:
		Field* m_property;

	public:
		StaticProperty(Field* property, Type* owner = nullptr) : Property(owner), m_property(property) {}
		Type* type() const override;

		bool assign(void* object, const Markup::ValueDesc& value) const override
		{
			return Property::assign(m_property, StaticProperty::type(), value);
		}
	};

	class Type
	{
	private:
		Type* m_parent;
		FlatMap<Name, Property*> m_properties;

	protected:
		Type(Type* parent);
		virtual ~Type();

		Type& bind(Name name, Property* prop);

	public:
		virtual void* factory() const       = 0;
		virtual Type& destroy(void* object) = 0;

		inline Type* parent() const { return m_parent; }
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
				return NativeType<Super>::instance();
			}
			else
			{
				return nullptr;
			}
		}

	private:
		NativeType(Type* parent = nullptr) : Type(parent) {}

	public:
		static NativeType* instance()
		{
			static NativeType type(resolve_parent());
			return &type;
		}

		void* factory() const override { return trx_new T(); }
		NativeType& destroy(void* object) override { trinex_this_return(trx_delete static_cast<T*>(object)); }

		template<typename Field>
		NativeType& bind(Name name, Field T::* field)
		{
			Type::bind(name, trx_new MemberProperty(field, this));
			return *this;
		}

		template<typename Field>
		NativeType& bind(Name name, Field* field)
		{
			Type::bind(name, trx_new StaticProperty(field, this));
			return *this;
		}
	};

	template<typename Field, typename Instance>
	Type* MemberProperty<Field, Instance>::type() const
	{
		return NativeType<Field>::instance();
	}

	template<typename Field>
	Type* StaticProperty<Field>::type() const
	{
		return NativeType<Field>::instance();
	}

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
}// namespace Trinex::UI::Refl
