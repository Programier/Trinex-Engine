#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/etl/pair.hpp>
#include <Core/types/name.hpp>
#include <UI/types.hpp>


namespace Trinex::UI
{
	class Type final
	{
	private:
		struct FieldBase {
			FieldBase const* owner;
			void* address;
			Type* type;
		};

		template<typename T>
		struct Field : public FieldBase {
			using FieldBase::FieldBase;

			inline T* ptr() const { return static_cast<T*>(address); }
			inline T& ref() const { return *ptr(); }
		};

		template<typename T>
		struct MemberTraits;

		template<typename FieldType, typename InstanceType>
		struct MemberTraits<FieldType InstanceType::*> {
			static inline Field<FieldType> field(const FieldBase& owner, FieldType InstanceType::* prop)
			{
				Field<FieldType> result;
				result.owner   = &owner;
				result.address = &(static_cast<InstanceType*>(owner.address)->*prop);
				result.type    = Type::instance<FieldType>();
				return result;
			}
		};

		template<typename FieldType>
		struct MemberTraits<FieldType*> {
			static inline Field<FieldType> field(const FieldBase& owner, FieldType* prop)
			{
				Field<FieldType> result;
				result.owner   = &owner;
				result.address = prop;
				result.type    = Type::instance<FieldType>();
				return result;
			}
		};

		struct Property {
			using Setter = bool (*)(const FieldBase& owner, const Property& prop, const Markup::ValueDesc& value);

			Type* type;
			Setter setter;
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


	private:
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

		static bool assign(const FieldBase& field, const Markup::ValueDesc& value);
		static bool assign(const Field<bool>& field, const Markup::ValueDesc& value);
		static bool assign(const Field<i32>& field, const Markup::ValueDesc& value);
		static bool assign(const Field<f32>& field, const Markup::ValueDesc& value);
		static bool assign(const Field<String>& field, const Markup::ValueDesc& value);

		template<typename Prop, typename Instance>
		inline Type* property_type(Prop Instance::* prop) const
		{
			trinex_assert(prop && type_id<Instance>() == m_id);
			return Type::instance<Prop>();
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
		Type& bind(Name name)
		{
			auto setter = +[](const FieldBase& object, const Property& property, const Markup::ValueDesc& value) {
				using Traits = MemberTraits<decltype(prop)>;
				return Type::assign(Traits::field(object, prop), value);
			};

			return property(name, property_type(prop), setter);
		}

		Pair<void*, Type*> property(void* object, Name* path, usize size);
		bool property(void* object, Name name, const Markup::ValueDesc& value);

		inline void* create() { return m_factory(); }
		inline Type& destroy(void* object) { trinex_this_return(m_destroy(object)); }
		inline Type* parent() const { return m_parent; }
		inline Pair<void*, Type*> property(void* object, Name path) { return property(object, &path, 1); }
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
