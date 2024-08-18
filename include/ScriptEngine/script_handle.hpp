#pragma once
#include <Core/engine_types.hpp>

class asITypeInfo;

namespace Engine
{
	class ENGINE_EXPORT ScriptPointer
	{
	private:
		mutable int_t m_refs;
		void* m_address;
		int_t m_type_id;
		mutable bool m_gc_flag;

		ScriptPointer();

	public:
		static ScriptPointer* create(asITypeInfo* ti);
		static ScriptPointer* create(asITypeInfo* ti, void* address);

		void add_ref() const;
		void release() const;

		void* address() const;
		int_t type_id() const;
		bool is_null() const;

		template<typename T>
		T* as() const
		{
			return reinterpret_cast<T*>(address());
		}
	};
}// namespace Engine
