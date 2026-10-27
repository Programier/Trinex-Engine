#pragma once
#include <Core/ref.hpp>

#include <Core/ref_counted.generated.hpp>

namespace Trinex
{
	struct WeakEntry;

	trinex_reflect()
	class trinex_api RefCounted
	{
		trinex_reflection_body();

	private:
		mutable u32 m_ref_count  = 1;
		mutable u32 m_weak_index = 0xFFFFFFFF;

	private:
		static WeakEntry* weak_entry(RefCounted* object) noexcept;
		static void add_weak_ref(WeakEntry* entry) noexcept;
		static void release_weak_ref(WeakEntry* entry) noexcept;
		static RefCounted* weak_ptr(WeakEntry* entry) noexcept;
		static RefCounted* lock_weak(WeakEntry* entry) noexcept;

		void release_weak_entry() const noexcept;

	protected:
		RefCounted() noexcept = default;

		virtual ~RefCounted();
		virtual void destroy() noexcept { trx_delete_inline(this); }

	public:
		trinex_non_copyable(RefCounted);
		trinex_non_moveable(RefCounted);

		void add_ref() const noexcept;
		void release() const noexcept;
		u32 references() const noexcept;

		template<typename>
		friend class Ref;

		template<typename>
		friend class WeakRef;

		friend class WeakRefBase;
	};

	template<typename T>
	class ExternalLifetime : public T
	{
	protected:
		using T::T;

		void destroy() noexcept final override
		{
			trinex_unreachable_msg("Externally-owned RefCounted object reached zero references");
		}
	};
} // namespace Trinex
