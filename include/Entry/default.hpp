#pragma once
#include <Core/entry.hpp>

#include <Entry/default.generated.hpp>

namespace Trinex
{
	trinex_reflect()
	class DefaultEntry : public EntryPoint
	{
		trinex_reflection_body();

	public:
		i32 execute() override;
	};
} // namespace Trinex
