#pragma once

#include <Core/object.hpp>


namespace Engine
{
	class ENGINE_EXPORT EntryPoint : public Object
	{
		trinex_class(EntryPoint, Object);

	public:
		virtual i32 execute();
	};
}// namespace Engine
