#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Systems/engine_system.hpp>
#include <chrono>

#include <Window/config.hpp>
#include <Window/window_manager.hpp>

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

    implement_engine_class_default_init(BaseEngine);

    FORCE_INLINE std::chrono::high_resolution_clock::time_point current_time_point()
    {
        return std::chrono::high_resolution_clock::now();
    }

    std::chrono::high_resolution_clock::time_point start_time;

    BaseEngine::BaseEngine()
    {}

    int_t BaseEngine::init()
    {
        start_time    = current_time_point();
        m_frame_index = 0;
        m_prev_time   = 0.f;
        EngineSystem::new_system<EngineSystem>();

        if (rhi)
        {
            WindowManager::instance()->create_client(WindowManager::instance()->main_window(), global_window_config.client);
        }
        return 0;
    }

    int_t BaseEngine::update()
    {
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


        auto& viewports = RenderViewport::viewports();

        for (auto& viewport : viewports)
        {
            viewport->update(m_delta_time);
        }

        render_thread()->wait_all();

        render_thread()->insert_new_task<BeginRenderCommand>();

        for (auto& viewport : viewports)
        {
            viewport->render();
        }

        render_thread()->insert_new_task<EndRenderCommand>();

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