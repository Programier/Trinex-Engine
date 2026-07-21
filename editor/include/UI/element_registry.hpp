#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/types/name.hpp>
#include <UI/types.hpp>


namespace Trinex::UI
{
	class Type final
	{
	private:
		template<typename T>
		struct MemberTraits;

		template<typename FieldType, typename InstanceType>
		struct MemberTraits<FieldType InstanceType::*> {
			using Instance = InstanceType;
			using Field    = FieldType;
		};

		template<typename T>
		static void* type_id()
		{
			static i32 value = 0;
			return &value;
		}

		template<typename T>
		static void* type_factory()
		{
			return trx_new T();
		}

		template<typename T>
		static void type_destroy(void* object)
		{
			trx_delete static_cast<T*>(object);
		}

		struct Property {
			using Setter = bool (*)(void* object, Type* type, const Markup::ValueDesc& value);

			Type* type;
			Setter setter;
		};

		void* m_id;
		Type* m_parent;
		void* (*m_factory)();
		void (*m_destroy)(void* object);
		FlatMap<Name, Property> m_properties;

	private:
		Type(void* id, void* (*factory)(), void (*destroy)(void* object))
		    : m_id(id), m_factory(factory), m_destroy(destroy), m_parent(nullptr)
		{}

		Type& property(Name name, Type* type, Property::Setter setter);

		static bool assign(void* dst, Type* type, const Markup::ValueDesc& value);
		static bool assign(bool* dst, Type* type, const Markup::ValueDesc& value);
		static bool assign(i32* dst, Type* type, const Markup::ValueDesc& value);
		static bool assign(f32* dst, Type* type, const Markup::ValueDesc& value);
		static bool assign(String* dst, Type* type, const Markup::ValueDesc& value);
		static bool assign(Markup::LocalizationKey* dst, Type* type, const Markup::ValueDesc& value);
		static bool assign(Markup::BindingPath* dst, Type* type, const Markup::ValueDesc& value);
		static bool assign(Markup::Identifier* dst, Type* type, const Markup::ValueDesc& value);
		static inline bool assign(Markup::Null* dst, Type* type, const Markup::ValueDesc& value) { return true; }

		template<typename Prop, typename Instance>
		inline Type* property_type(Prop Instance::* prop) const
		{
			trinex_assert(prop && type_id<Instance>() == m_id);
			return Type::instance<Instance>();
		}

	public:
		template<typename T>
		static Type* instance()
		{
			static Type type(type_id<T>(), type_factory<T>, type_destroy<T>);
			trinex_assert(type.m_id == type_id<T>());
			return &type;
		}

		template<typename T>
		inline Type* parent()
		{
			m_parent = instance<T>();
			return m_parent;
		}

		template<auto prop>
		Type& property(Name name)
		{
			using Instance = MemberTraits<decltype(prop)>::Instance;
			using Field    = MemberTraits<decltype(prop)>::Field;

			auto setter = +[](void* object, Type* type, const Markup::ValueDesc& value) {
				Field* field = &(static_cast<Instance*>(object)->*prop);
				return Type::assign(field, type, value);
			};

			return property(name, property_type(prop), setter);
		}

		bool property(void* object, Name name, const Markup::ValueDesc& value);

		inline void* create() { return m_factory(); }
		inline Type& destroy(void* object) { trinex_this_return(m_destroy(object)); }
		inline Type* parent() const { return m_parent; }
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
		Type* bind(Name name)
		{
			if (m_types.contains(name))
				return nullptr;

			Type* element = Type::instance<T>();
			m_types.insert({name, element});
			return element;
		}

		inline void unbind(Name name) { m_types.erase(name); }

		inline Type* find(Name name) const
		{
			auto it = m_types.find(name);
			return it == m_types.end() ? nullptr : it->second;
		}

		inline const Container& types() const { return m_types; }
	};
}// namespace Trinex::UI
