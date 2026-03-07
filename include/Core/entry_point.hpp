#pragma once

#include <Core/object.hpp>


namespace Trinex
{
	class ENGINE_EXPORT EntryPoint : public Object
	{
		trinex_class(EntryPoint, Object);

	public:
		virtual i32 execute();
	};
}// namespace Trinex
