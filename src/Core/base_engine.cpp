#include <Core/base_engine.hpp>
#include <Core/reflection/class.hpp>
#include <Core/config_manager.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/profiler.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Systems/engine_system.hpp>
#include <Window/window_manager.hpp>
#include <chrono>

namespace Engine
{
	struct BeginRenderCommand : public ExecutableObject {
		int_t execute() override
		{
			rhi->begin_render();
			return sizeof(BeginRenderCommand);
		}
	};

	struct EndRenderCommand : public ExecutableObject {
		int_t execute() override
		{
			rhi->end_render();
			return sizeof(EndRenderCommand);
		}
	};

	implement_engine_class_default_init(BaseEngine, 0);

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
	}

	int_t BaseEngine::init()
	{
		EngineSystem::new_system<EngineSystem>();
		return 0;
	}

	int_t BaseEngine::update()
	{
		trinex_profile_frame_mark();
		trinex_profile_cpu();

		auto current_time = time_seconds();

		m_delta_time = current_time - m_prev_time;
		m_prev_time  = current_time;

		++m_frame_index;

		GarbageCollector::update(m_delta_time);

		if (auto instance = EngineSystem::instance())
		{
			instance->update(m_delta_time);
			instance->wait();
		}

		if (!is_requesting_exit())
		{
			auto& viewports    = RenderViewport::viewports();
			Size2D max_vp_size = {0.f, 0.f};

			for (size_t i = 0; i < viewports.size(); ++i)
			{
				auto viewport = viewports[i];

				if (viewport->is_active())
				{
					viewport->update(m_delta_time);
					max_vp_size = glm::max(max_vp_size, viewport->size());
				}
			}

			begin_render();

			SceneRenderTargets::instance()->initialize(max_vp_size);

			for (size_t i = 0; i < viewports.size(); ++i)
			{
				auto viewport = viewports[i];

				if (viewport->is_active())
					viewport->render();
			}

			end_render();
		}
		return 0;
	}

	BaseEngine& BaseEngine::begin_render()
	{
		render_thread()->wait_all();
		render_thread()->insert_new_task<BeginRenderCommand>();
		return *this;
	}

	BaseEngine& BaseEngine::end_render()
	{
		render_thread()->insert_new_task<EndRenderCommand>();
		return *this;
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
		m_flags(Flag::IsRequestExit, 1);
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
		m_flags(Flag::IsInitied);
		return *this;
	}

	float BaseEngine::delta_time() const
	{
		return m_delta_time;
	}

	float BaseEngine::time_seconds() const
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(current_time_point() - start_time).count();
	}

	Index BaseEngine::frame_index() const
	{
		return m_frame_index;
	}

	ENGINE_EXPORT BaseEngine* engine_instance = nullptr;
}// namespace Engine
