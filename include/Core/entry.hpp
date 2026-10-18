#pragma once

#include <Core/object.hpp>

namespace Trinex
{
	class trinex_api EntryPoint : public Object
	{
		trinex_class(EntryPoint, Object);

	public:
		virtual i32 execute();
	};
}// namespace Trinex
