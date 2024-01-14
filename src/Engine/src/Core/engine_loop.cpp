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
        RHI* _M_rhi;

        BeginRenderCommand(RHI* rhi) : _M_rhi(rhi)
        {}

        int_t execute() override
        {
            _M_rhi->begin_render();
            return sizeof(BeginRenderCommand);
        }
    };

    struct EndRenderCommand : public ExecutableObject {
        RHI* _M_rhi;

        EndRenderCommand(RHI* rhi) : _M_rhi(rhi)
        {}

        int_t execute() override
        {
            _M_rhi->end_render();
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
            static constexpr float smoothing_factor = 0.05;

            float prev_time    = 1.0f / static_cast<float>(engine_config.fps_limit);
            float current_time = 0.0f;
            float dt           = 0.0f;

            while (!is_requesting_exit())
            {
                {
                    float wait_time = (1.0f / static_cast<float>(engine_config.fps_limit)) - (time_seconds() - prev_time);

                    if (wait_time > 0)
                    {
                        Thread::sleep_for(wait_time);
                    }
                }

                current_time = time_seconds();
                dt           = smoothing_factor * (current_time - prev_time) + (1 - smoothing_factor) * dt;
                prev_time    = current_time;

                engine_system->update(dt);
                engine_system->wait();

                // Update Viewports

                auto& viewports        = RenderViewport::viewports();
                size_t viewports_count = viewports.size();

                for (size_t i = 0; i < viewports_count; ++i)
                {
                    viewports[i]->update(dt);
                }

                render_thread->wait_all();

                for (size_t i = 0, count = glm::min(viewports.size(), viewports_count); i < count; ++i)
                {
                    viewports[i]->prepare_render();
                }

                render_thread->insert_new_task<BeginRenderCommand>(_M_rhi);

                for (size_t i = 0, count = glm::min(viewports.size(), viewports_count); i < count; ++i)
                {
                    viewports[i]->render();
                }

                render_thread->insert_new_task<EndRenderCommand>(_M_rhi);

                ++_M_frame_index;
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
