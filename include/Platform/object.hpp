#pragma once

namespace Trinex::Platform
{
	class ENGINE_EXPORT Object
	{
	private:
		static void static_release_internal(Object* object);

	protected:
		usize m_references;

	public:
		Object(usize init_ref_count = 1);
		virtual void add_reference();
		virtual void release();
		virtual void destroy() = 0;
		usize references() const;
		virtual ~Object();

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
}// namespace Trinex::Platform
