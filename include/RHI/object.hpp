#pragma once
#include <Core/ref_counted.hpp>

namespace Trinex
{
	class RHI;
	class RHIGarbageGroup;

	class trinex_api RHIObject : public RefCounted
	{
	protected:
		virtual RHIGarbageGroup* garbage_group() const;

	public:
		virtual void destroy() noexcept;
		virtual RHI* device() const = 0;
		virtual ~RHIObject();

		template<typename T>
		T* as()
		{
			return static_cast<T*>(this);
		}

		template<typename T>
		const T* as() const
		{
			return static_cast<const T*>(this);
		}
	};

	template<typename T>
	class RHIDirectDestroy : public T
	{
	protected:
		void destroy() noexcept override { trx_delete this; }

	public:
		using T::T;
	};
} // namespace Trinex
