#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Systems/engine_system.hpp>

namespace Engine
{

    struct BeginRenderCommand : public ExecutableObject {
        RHI* m_rhi;

        BeginRenderCommand(RHI* rhi) : m_rhi(rhi)
        {}

        int_t execute() override
        {
            m_rhi->begin_render();
            return sizeof(BeginRenderCommand);
        }
    };

    struct EndRenderCommand : public ExecutableObject {
        RHI* m_rhi;

        EndRenderCommand(RHI* rhi) : m_rhi(rhi)
        {}

        int_t execute() override
        {
            m_rhi->end_render();
            return sizeof(EndRenderCommand);
        }
    };

    int_t EngineInstance::launch()
    {

        EngineSystem* engine_system = System::new_system<EngineSystem>();
        Thread* render_thread       = thread(ThreadType::RenderThread);

        engine_system->create_systems_from_config();

        if (engine_system->subsystems().empty())
        {
            error_log("Engine", "No systems found! Please, add systems before call '%s' method!", __FUNCTION__);
            return -1;
        }

        // Sort all systems
        engine_system->sort_subsystems();

        try
        {
            static constexpr float smoothing_factor = 0.3;

            float prev_time    = 1.0f / static_cast<float>(engine_config.fps_limit);
            float current_time = 0.0f;
            m_delta_time       = 0.0f;

            while (!is_requesting_exit())
            {
                current_time = time_seconds();
                m_delta_time = glm::mix(m_delta_time, current_time - prev_time, smoothing_factor);
                prev_time    = current_time;

                m_current_gc_stage = Object::collect_garbage(m_current_gc_stage);

                engine_system->update(m_delta_time);
                engine_system->wait();

                // Update Viewports

                auto& viewports = RenderViewport::viewports();

                for (auto& viewport : viewports)
                {
                    viewport->update(m_delta_time);
                }

                render_thread->wait_all();

                render_thread->insert_new_task<BeginRenderCommand>(m_rhi);

                for (auto& viewport : viewports)
                {
                    viewport->render();
                }

                render_thread->insert_new_task<EndRenderCommand>(m_rhi);

                ++m_frame_index;
            }

            return 0;
        }
        catch (const std::exception& e)
        {
            error_log("Engine", "%s", e.what());
            return -1;
        }
    }
}// namespace Engine
