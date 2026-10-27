#pragma once
#include <Core/deprecated/object.hpp>

namespace Trinex
{
	template<typename Parent>
	class EngineResource : public Parent
	{
	public:
		using Parent::Parent;

		EngineResource() { DeprecatedObject::flags |= DeprecatedObject::Flags::StandAlone; }
	};
} // namespace Trinex
