#pragma once
#include <Entry/engine.hpp>

#include <Entry/debug_engine.generated.hpp>

namespace Trinex
{
	class RenderViewport;

	trinex_reflect()
	class DebugEngineEntry : public EngineEntry
	{
		trinex_reflection_body();

	private:
		Ref<RenderViewport> m_viewport;

	private:
		void on_start();

	public:
		i32 execute() override;
	};
} // namespace Trinex
