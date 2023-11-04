#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/singletone.hpp>
#include <Graphics/global_uniform_buffer.hpp>
#include <Graphics/rhi.hpp>
#include <Platform/thread.hpp>
#include <Systems/engine_system.hpp>

namespace Engine
{

    struct BeginRenderTask : public ExecutableObject {
        RHI* rhi;

        BeginRenderTask(RHI* rhi) : rhi(rhi)
        {}

        int_t execute() override
        {
            rhi->begin_render();
            return 0;
        }
    };

    struct EndRenderTask : public ExecutableObject {
        RHI* rhi;

        EndRenderTask(RHI* rhi) : rhi(rhi)
        {}

        int_t execute() override
        {
            rhi->end_render();
            rhi->swap_buffer();
            return 0;
        }
    };


    EngineSystem& EngineSystem::create()
    {
        Super::create();

        for (const String& system_name : engine_config.systems)
        {
            System* system = System::new_system_by_name(system_name);
            if (system && system->owner() == nullptr)
            {
                register_subsystem(system);
            }
        }

        return *this;
    }

    EngineSystem& EngineSystem::update(float dt)
    {
        Super::update(dt);
        return *this;
    }

    implement_class(EngineSystem, "Engine");
    implement_default_initialize_class(EngineSystem);
}// namespace Engine
