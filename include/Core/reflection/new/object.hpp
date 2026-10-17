#pragma once
#include <Core/ref_counted.hpp>
#include <Core/types/name.hpp>

namespace Trinex::Refl::New
{
	class ENGINE_EXPORT Object : public RefCounted
	{
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
		bool owner(Object* object);

		virtual Object* find(Name name) const;

		inline Name name() const { return m_name; }
		inline Ref<Object> owner() const { return m_owner.lock(); }
		inline u64 flags() const { return m_flags; }
		inline bool is_deprecated() const { return has_flags(Deprecated); }
		inline bool is_editor_only() const { return has_flags(EditorOnly); }
		inline bool is_hidden() const { return has_flags(Hidden); }
	};
}// namespace Trinex::Refl::New
