#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/tickable.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_viewport.hpp>
#include <Input/event_system.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <Window/window_manager.hpp>
#include <chrono>

namespace Trinex
{
	namespace
	{
		class EngineQuitListener final : public EventListener
		{
		public:
			EventDispatchResult on_event(RoutedEvent& event) override
			{
				if (event.header.type_id == EventTypeIds::Quit && engine_instance)
				{
					engine_instance->request_exit();
					event.mark_handled();
				}

				return event.result;
			}
		};

		FORCE_INLINE std::chrono::high_resolution_clock::time_point current_time_point()
		{
			return std::chrono::high_resolution_clock::now();
		}

		static EngineQuitListener quit_listener;
		std::chrono::high_resolution_clock::time_point start_time;
	}// namespace

	trinex_implement_engine_class_default_init(BaseEngine, 0);


	BaseEngine::BaseEngine()
	{
		start_time    = current_time_point();
		m_frame_index = 0;
		m_prev_time   = 0.f;

		flags.set(Flags::StandAlone);
		flags.remove(Flags::IsAvailableForGC);

		if (EventSystem* system = EventSystem::instance())
		{
			system->dispatcher().add_listener(EventTypeIds::Quit, &quit_listener);
		}
	}

	i32 BaseEngine::init()
	{
		return 0;
	}

	i32 BaseEngine::update()
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

		Tickable::for_each_begin_frame();
		Tickable::for_each_update(m_delta_time);

		if (RHI* rhi = RHI::instance())
		{
			rhi->update(m_delta_time);
		}

		if (!is_requesting_exit())
		{
			auto& viewports = RenderViewport::viewports();

			for (usize i = 0; i < viewports.size(); ++i)
			{
				trinex_profile_cpu_n("RenderViewport::update");
				auto viewport = viewports[i];
				viewport->update(m_delta_time);
			}
		}

		Tickable::for_each_end_frame();
		return 0;
	}

	i32 BaseEngine::terminate()
	{
		m_is_shutting_down = true;
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
		m_is_requesting_exit = true;
		return *this;
	}

	bool BaseEngine::is_requesting_exit() const
	{
		return m_is_requesting_exit;
	}

	bool BaseEngine::is_shuting_down() const
	{
		return m_is_shutting_down;
	}

	float BaseEngine::time_seconds() const
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(current_time_point() - start_time).count();
	}

	ENGINE_EXPORT BaseEngine* engine_instance = nullptr;
}// namespace Trinex
