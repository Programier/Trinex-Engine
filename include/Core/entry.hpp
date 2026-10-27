#pragma once

#include <Core/deprecated/object.hpp>

namespace Trinex
{
	class trinex_api EntryPoint : public DeprecatedObject
	{
		trinex_class(EntryPoint, DeprecatedObject);

	public:
		virtual i32 execute();
	};
} // namespace Trinex
