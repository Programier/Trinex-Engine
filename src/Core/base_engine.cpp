#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_viewport.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/window_manager.hpp>
#include <chrono>

namespace Engine
{
	trinex_implement_engine_class_default_init(BaseEngine, 0);

	FORCE_INLINE std::chrono::high_resolution_clock::time_point current_time_point()
	{
		return std::chrono::high_resolution_clock::now();
	}

	std::chrono::high_resolution_clock::time_point start_time;

	BaseEngine::BaseEngine()
	{
		start_time    = current_time_point();
		m_frame_index = 0;
		m_prev_time   = 0.f;

		flags(StandAlone, true);
		flags(IsAvailableForGC, false);

		System::system_of<EventSystem>()->add_listener(EventType::Quit, [](const Event&) {
			if (engine_instance)
			{
				engine_instance->request_exit();
			}
		});
	}

	int_t BaseEngine::init()
	{
		EngineSystem::system_of<EngineSystem>();
		return 0;
	}

	int_t BaseEngine::update()
	{
		trinex_profile_frame_mark();
		trinex_profile_cpu_n("BaseEngine::update");

		auto current_time = time_seconds();

		m_delta_time = current_time - m_prev_time;
		m_prev_time  = current_time;
		++m_frame_index;

		GarbageCollector::update(m_delta_time);

		StackByteAllocator::reset();
		FrameByteAllocator::reset();

		if (auto instance = EngineSystem::instance())
		{
			trinex_profile_cpu_n("Systems::update");
			instance->update(m_delta_time);
			instance->wait();
		}

		if (!is_requesting_exit())
		{
			auto& viewports = RenderViewport::viewports();

			for (size_t i = 0; i < viewports.size(); ++i)
			{
				trinex_profile_cpu_n("RenderViewport::update");
				auto viewport = viewports[i];
				viewport->update(m_delta_time);
			}
		}
		return 0;
	}

	int_t BaseEngine::terminate()
	{
		m_flags(Flag::IsShuttingDown, 1);
		return 0;
	}

	StringView BaseEngine::application_name() const
	{
		return "Trinex Example";
	}

	float BaseEngine::max_tick_rate() const
	{
		return 60.f;
	}

	float BaseEngine::gamma() const
	{
		return 1.f;
	}

	BaseEngine& BaseEngine::request_exit()
	{
		m_flags(Flag::IsRequestExit, true);
		return *this;
	}

	bool BaseEngine::is_requesting_exit() const
	{
		return m_flags(Flag::IsRequestExit);
	}

	bool BaseEngine::is_shuting_down() const
	{
		return m_flags(Flag::IsShuttingDown);
	}

	bool BaseEngine::is_inited() const
	{
		return m_flags(Flag::IsInitied);
	}

	BaseEngine& BaseEngine::make_inited()
	{
		m_flags(Flag::IsInitied, true);
		return *this;
	}

	float BaseEngine::time_seconds() const
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(current_time_point() - start_time).count();
	}

	ENGINE_EXPORT BaseEngine* engine_instance = nullptr;
}// namespace Engine
