#pragma once
#include <Core/ref_counted.hpp>
#include <Core/reflection/new/of.hpp>
#include <Core/types/name.hpp>

#include <Core/reflection/new/object.generated.hpp>

namespace Trinex::Refl::New
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
		Object(Name name, u64 flags = 0, Object* owner = nullptr);

		virtual Class* reflection() const;
		virtual Object* resolve(const Class* base);
		virtual const Object* resolve(const Class* base) const;
		virtual Object* find(Name name, const Class* base = nullptr) const;

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

		inline Name name() const { return m_name; }

		inline Ref<Object> owner() const { return m_owner.lock(); }

		inline u64 flags() const { return m_flags; }

		inline bool is_deprecated() const { return has_flags(Deprecated); }

		inline bool is_editor_only() const { return has_flags(EditorOnly); }

		inline bool is_hidden() const { return has_flags(Hidden); }
	};
} // namespace Trinex::Refl::New
