#include <Core/ref.hpp>
#include <Core/ref_counted.hpp>

namespace Trinex
{
	void RefBase::add_ref(void* object)
	{
		static_cast<RefCounted*>(object)->add_ref();
	}

	void RefBase::release(void* object)
	{
		static_cast<RefCounted*>(object)->release();
	}

	void* WeakRefBase::entry(RefCounted* object) noexcept
	{
		return RefCounted::weak_entry(object);
	}

	void WeakRefBase::add_ref(void* entry) noexcept
	{
		RefCounted::add_weak_ref(static_cast<WeakEntry*>(entry));
	}

	void WeakRefBase::release(void* entry) noexcept
	{
		RefCounted::release_weak_ref(static_cast<WeakEntry*>(entry));
	}

	RefCounted* WeakRefBase::weak_ptr(void* entry) noexcept
	{
		return RefCounted::weak_ptr(static_cast<WeakEntry*>(entry));
	}

	RefCounted* WeakRefBase::lock_weak(void* entry) noexcept
	{
		return RefCounted::lock_weak(static_cast<WeakEntry*>(entry));
	}

} // namespace Trinex
