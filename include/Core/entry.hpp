#pragma once
#include <Core/object.hpp>

#include <Core/entry.generated.hpp>

namespace Trinex
{
	trinex_reflect()
	class trinex_api EntryPoint : public Object
	{
		trinex_reflection_body();

	public:
		trinex_function()
		virtual i32 execute();
	};
} // namespace Trinex
