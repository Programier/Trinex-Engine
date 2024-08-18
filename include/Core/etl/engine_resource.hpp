#pragma once
#include <Core/object.hpp>

namespace Engine
{
	template<typename Parent>
	class EngineResource : public Parent
	{
	public:
		using Parent::Parent;

		EngineResource()
		{
			Object::flags(Object::StandAlone, true);
		}
	};
}// namespace Engine
