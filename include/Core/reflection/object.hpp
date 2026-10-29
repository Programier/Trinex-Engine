#pragma once
#include <Core/ref_counted.hpp>
#include <Core/reflection/indirect_resolver.hpp>
#include <Core/reflection/of.hpp>
#include <Core/types/name.hpp>

#include <Core/reflection/object.generated.hpp>

namespace Trinex::Refl
{
	class Class;

	trinex_reflect()
	class trinex_api Object : public RefCounted
	{
		trinex_reflection_body();

	private:
		WeakRef<Object> m_owner;
		u64 m_flags;
		Name m_name;

	protected:
		bool has_flags(u64 flags) const { return (m_flags & flags) == flags; }

		bool has_any_flags(u64 flags) const { return (m_flags & flags) != 0; }

		virtual bool register_child(Object* object);
		virtual bool unregister_child(Object* object);

	public:
		enum Flags : u64
		{
			None       = 0,
			Deprecated = 1ull << 56,
			EditorOnly = 1ull << 57,
			Hidden     = 1ull << 58,
		};

		static Object* root();

	public:
		Object(Name name, u64 flags = 0);

		virtual Class* reflection() const;
		virtual Object* resolve(const Class* base);
		virtual const Object* resolve(const Class* base) const;
		virtual Object* find(Name name, const Class* base = nullptr) const;
		Object* lookup(StringView path, const Class* base = nullptr) const;

		bool owner(Object* object);
		bool is(const Class* base) const;

		template<typename T>
		bool is() const
		{
			return is(class_of<T>());
		}

		template<typename T>
		T* cast()
		{
			return is<T>() ? static_cast<T*>(this) : nullptr;
		}

		template<typename T>
		const T* cast() const
		{
			return is<T>() ? static_cast<const T*>(this) : nullptr;
		}

		template<typename T>
		static T* cast(Object* object)
		{
			if (object == nullptr)
				return nullptr;

			return object->cast<T>();
		}

		template<typename T>
		static const T* cast(const Object* object)
		{
			if (object == nullptr)
				return nullptr;

			return object->cast<T>();
		}

		template<typename T>
		T* resolve()
		{
			return static_cast<T*>(resolve(class_of<T>()));
		}

		template<typename T>
		const T* resolve() const
		{
			return static_cast<const T*>(resolve(class_of<T>()));
		}

		template<typename T>
		T* find(Name name) const
		{
			return static_cast<T*>(find(name, class_of<T>()));
		}

		template<typename T>
		T* lookup(StringView path) const
		{
			return static_cast<T*>(lookup(path, class_of<T>()));
		}

		inline Name name() const { return m_name; }

		inline Ref<Object> owner() const { return m_owner.lock(); }

		inline u64 flags() const { return m_flags; }

		inline bool is_deprecated() const { return has_flags(Deprecated); }

		inline bool is_editor_only() const { return has_flags(EditorOnly); }

		inline bool is_hidden() const { return has_flags(Hidden); }
	};

	template<>
	struct IndirectResolver<Object>
	{
		static trinex_api usize parse(Object*& result, Refl::Class* base, StringView value);
		static trinex_api bool format(const Object* object, String& value);

		template<typename T>
		static usize parse(T*& result, StringView value)
		{
			Object* object     = nullptr;
			const usize parsed = parse(object, class_of<T>(), value);

			if (parsed != 0)
				result = static_cast<T*>(object);

			return parsed;
		}
	};

	template<typename T>
	    requires(trinex_is_base_of(Object, T) && !trinex_is_same(Object, T))
	struct IndirectResolver<T> : public IndirectResolver<Object>
	{
	};
} // namespace Trinex::Refl
