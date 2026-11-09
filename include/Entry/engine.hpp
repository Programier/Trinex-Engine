#pragma once
#include <Core/entry.hpp>

#include <Entry/engine.generated.hpp>

namespace Trinex
{
	class Engine;

	trinex_reflect()
	class EngineEntry : public EntryPoint
	{
		trinex_reflection_body();

	public:
		virtual Ref<Engine> create_engine();
		i32 execute() override;
	};
} // namespace Trinex
