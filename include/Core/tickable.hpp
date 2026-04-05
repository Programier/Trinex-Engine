#pragma once
#include <Core/etl/registry.hpp>

namespace Trinex
{
	class ENGINE_EXPORT TickableObject : public Registry<TickableObject>
	{
		trinex_registry(TickableObject);

	public:
		virtual TickableObject& update(float dt);
		virtual bool is_tickable() const;
		virtual bool is_tickable_when_paused() const;
	};
}// namespace Trinex
