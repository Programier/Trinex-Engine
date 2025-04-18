#pragma once
#include <Core/reflection/object.hpp>

namespace Engine::Refl
{
	class ENGINE_EXPORT ScopedType : public Object
	{
		declare_reflect_type(ScopedType, Object);

	protected:
		Map<String, Object*> m_childs;
		mutable ushort_t m_lock_count = 0;

		ScopedType& unregister_subobject(Object* subobject) override;
		ScopedType& register_subobject(Object* subobject) override;

		~ScopedType();

	private:
		void lock() const;
		void unlock() const;

	public:
		struct Locker final {
		private:
			const ScopedType* const m_object;

		public:
			FORCE_INLINE Locker(const ScopedType* const object) : m_object(object) { m_object->lock(); }

			~Locker() { m_object->unlock(); }
		};

		using Super::find;
		Object* find(StringView name, FindFlags flags = FindFlags::None) override;
		const Map<String, Object*>& childs() const;

		FORCE_INLINE bool is_locked() const { return m_lock_count > 0; }
	};
}// namespace Engine::Refl
