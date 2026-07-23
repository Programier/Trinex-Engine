#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/types/name.hpp>
#include <UI/types.hpp>

namespace Trinex::UI
{
	class Document;
}

namespace Trinex::UI::Refl
{
	class Type;

	struct AssignHistory {
		const AssignHistory* prev;
		void* dst            = nullptr;
		const Type* dst_type = nullptr;

		const void* src      = nullptr;
		const Type* src_type = nullptr;
	};

	class Property
	{
	private:
		Type* m_owner;

	protected:
	public:
		inline Property(Type* owner = nullptr) : m_owner(owner) {}

		bool assign(void* object, const void* src, const Type* src_type, const AssignHistory* history = nullptr);

		virtual Type* type() const                             = 0;
		virtual void* resolve(void* address)                   = 0;
		virtual const void* resolve(const void* address) const = 0;
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

		void* resolve(void* address) override { return &(static_cast<Instance*>(address)->*m_property); }
		const void* resolve(const void* address) const override { return &(static_cast<const Instance*>(address)->*m_property); }
	};

	template<typename Field>
	class StaticProperty final : public Property
	{
	private:
		Field* m_property;

	public:
		StaticProperty(Field* property, Type* owner = nullptr) : Property(owner), m_property(property) {}
		Type* type() const override;

		void* resolve(void* address) override { return m_property; }
		const void* resolve(const void* address) const override { return m_property; }
	};

	class Type
	{
	public:
		using Resolver = bool (*)(void* dst, const void* src, const AssignHistory* history);

	private:
		Type* m_parent;
		FlatMap<Name, Property*> m_properties;
		FlatMap<const Type*, Resolver> m_resolvers;

	protected:
		Type(Type* parent);
		virtual ~Type();

		Type& bind(Name name, Property* prop);
		bool binding_path_resolver(void* dst, const void* src, const AssignHistory* history);

	public:
		virtual void* factory() const       = 0;
		virtual Type& destroy(void* object) = 0;

		Pair<void*, const Type*> resolve(void* address, const Name* names, usize count = 1) const;

		bool assign(void* object, Name property, const void* src, const Type* type, const AssignHistory* history = nullptr) const;
		bool assign(void* dst, const void* src, const Type* type, const AssignHistory* history = nullptr) const;
		virtual bool assign(void* dst, const void* src) const = 0;

		Property* property(Name name) const;
		inline Type* parent() const { return m_parent; }
		inline Pair<void*, const Type*> resolve(void* address, const Name& name) { return resolve(address, &name, 1); }

		Type& bind(Type* type, Resolver resolver);

		template<typename Source>
		Type& bind(Resolver resolver);

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
				return NativeType<Super>::instance();
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

			Type::bind(binding_path, [](void* dst, const void* src, const AssignHistory* history) -> bool {
				return NativeType<T>::instance()->binding_path_resolver(dst, src, history);
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
		    requires(etl::is_class_v<Instance>)
		NativeType& bind(Name name, Field Instance::* field)
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

		friend Document;
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

	template<typename Source>
	Type& Type::bind(Resolver resolver)
	{
		Type::bind(NativeType<Source>::instance(), resolver);
		return *this;
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
