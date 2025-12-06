#pragma once
#include <Core/etl/singletone.hpp>
#include <Systems/system.hpp>

namespace Engine
{
	class ENGINE_EXPORT LogicSystem : public Singletone<LogicSystem, System>
	{
		trinex_class(LogicSystem, System);

	protected:
		LogicSystem& create() override;

	public:
		LogicSystem& update(float dt) override;
		LogicSystem& shutdown() override;
		LogicSystem& wait() override;
		class Refl::Class* depends_on() const override;
		friend class Singletone<LogicSystem, System>;
	};
}// namespace Engine
