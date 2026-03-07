#pragma once
#include <Core/engine_types.hpp>

namespace Trinex
{
	class ENGINE_EXPORT RHIObject
	{
	private:
		static void static_release_internal(RHIObject* object);

	protected:
		usize m_references;

	public:
		RHIObject(usize init_ref_count = 1);
		virtual void add_reference();
		virtual void release();
		virtual void destroy() = 0;
		usize references() const;
		virtual ~RHIObject();

		template<typename T>
		static inline void static_release(T* object)
		{
			if (object)
			{
				static_release_internal(object);
			}
		}

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
}// namespace Trinex
