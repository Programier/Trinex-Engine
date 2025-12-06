#pragma once
#include <Core/etl/singletone.hpp>
#include <Systems/system.hpp>

namespace Engine
{
	class ENGINE_EXPORT EngineSystem : public Singletone<EngineSystem, System>
	{
		trinex_class(EngineSystem, System);

	protected:
		EngineSystem& create() override;

	public:
		EngineSystem& update(float dt) override;
		EngineSystem& create_systems_from_config();

		friend class Singletone<EngineSystem, System>;
	};
}// namespace Engine
