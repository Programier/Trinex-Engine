#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class ENGINE_EXPORT RHIObject
	{
	private:
		static void static_release_internal(RHIObject* object);

	protected:
		size_t m_references;

	public:
		RHIObject(size_t init_ref_count = 1);
		virtual void add_reference();
		virtual void release();
		virtual void destroy() = 0;
		size_t references() const;
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
}// namespace Engine
