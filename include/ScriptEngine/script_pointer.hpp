#pragma once
#include <Core/engine_types.hpp>

class asITypeInfo;

namespace Engine
{
	class ENGINE_EXPORT ScriptPointer
	{
	private:
		void* m_address = nullptr;

	public:
		ScriptPointer(void* address = nullptr);
		ScriptPointer(const ScriptPointer& other);
		ScriptPointer& operator=(const ScriptPointer& other);

		void* address() const;
		bool is_null() const;

		template<typename T>
		T* as() const
		{
			return reinterpret_cast<T*>(address());
		}
	};
}// namespace Engine
