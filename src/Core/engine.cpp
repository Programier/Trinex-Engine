#include <Core/engine.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/tickable.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_viewport.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <chrono>

namespace Trinex
{
	namespace
	{
		static Engine* s_instance = nullptr;
		FORCE_INLINE std::chrono::high_resolution_clock::time_point current_time_point()
		{
			return std::chrono::high_resolution_clock::now();
		}

		std::chrono::high_resolution_clock::time_point start_time;
	}// namespace

	trinex_implement_engine_class_default_init(Engine, 0);


	Engine::Engine()
	{
		start_time    = current_time_point();
		m_frame_index = 0;
		m_prev_time   = 0.f;

		flags.set(Flags::StandAlone);
		flags.remove(Flags::IsAvailableForGC);

		InputSystem::instance()->add_listener(nullptr, EventType::Quit, EventCode::Undefined,
		                                      [this](const InputEvent&) { return request_exit(), true; });
	}

	Engine* Engine::instance()
	{
		return s_instance;
	}

	i32 Engine::launch(int argc, char** argv)
	{
		return 0;
	}

	i32 Engine::init()
	{
		return 0;
	}

	i32 Engine::update()
	{
		trinex_profile_frame_mark();
		trinex_profile_cpu_n("Engine::update");

		auto current_time = time_seconds();

		m_delta_time = current_time - m_prev_time;
		m_prev_time  = current_time;
		++m_frame_index;
		InputSystem::instance()->begin_frame().update(m_delta_time);

		GarbageCollector::update(m_delta_time);

		StackByteAllocator::reset();
		FrameByteAllocator::reset();

		Tickable::for_each_begin_frame(m_frame_index);
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

		Tickable::for_each_end_frame(m_frame_index);
		return 0;
	}

	i32 Engine::terminate()
	{
		m_is_shutting_down = true;
		return 0;
	}

	StringView Engine::application_name() const
	{
		return "Trinex Example";
	}

	float Engine::max_tick_rate() const
	{
		return 60.f;
	}

	float Engine::gamma() const
	{
		return 1.f;
	}

	Engine& Engine::request_exit()
	{
		m_is_requesting_exit = true;
		return *this;
	}

	bool Engine::is_requesting_exit() const
	{
		return m_is_requesting_exit;
	}

	bool Engine::is_shuting_down() const
	{
		return m_is_shutting_down;
	}

	float Engine::time_seconds() const
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(current_time_point() - start_time).count();
	}

	ENGINE_EXPORT Engine* engine_instance = nullptr;
}// namespace Trinex
