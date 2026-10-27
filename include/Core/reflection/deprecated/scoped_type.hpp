#pragma once
#include <Core/reflection/deprecated/object.hpp>

namespace Trinex::Refl::Deprecated
{
	class trinex_api ScopedType : public Object
	{
		trinex_reflect_type(ScopedType, Object);

	protected:
		Map<String, Object*> m_childs;
		mutable u16 m_lock_count = 0;

		ScopedType& unregister_subobject(Object* subobject) override;
		ScopedType& register_subobject(Object* subobject) override;

		~ScopedType();

	private:
		void lock() const;
		void unlock() const;

	public:
		struct Locker final
		{
		private:
			const ScopedType* const m_object;

		public:
			inline Locker(const ScopedType* const object) : m_object(object) { m_object->lock(); }

			~Locker() { m_object->unlock(); }
		};

		using Super::find;
		Object* find(StringView name, FindFlags flags = FindFlags::Undefined) override;
		const Map<String, Object*>& childs() const;

		inline bool is_locked() const { return m_lock_count > 0; }
	};
} // namespace Trinex::Refl::Deprecated
