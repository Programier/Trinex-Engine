#pragma once
#include <Core/object.hpp>

namespace Trinex
{
	template<typename Parent>
	class EngineResource : public Parent
	{
	public:
		using Parent::Parent;

		EngineResource() { Object::flags |= Object::Flags::StandAlone; }
	};
}// namespace Trinex
