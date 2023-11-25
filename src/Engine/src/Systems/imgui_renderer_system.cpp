#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/imgui.hpp>
#include <Core/thread.hpp>
#include <Systems/imgui_system.hpp>
#include <Systems/render_system.hpp>
#include <Window/window.hpp>

namespace Engine
{

    class InitImGuiTask : public ExecutableObject
    {
    public:
        int_t execute() override
        {
            ImGuiRenderer::init();
            return sizeof(InitImGuiTask);
        }
    };


    class TerminateImGuiTask : public ExecutableObject
    {
    public:
        int_t execute() override
        {
            ImGuiRenderer::terminate();
            return sizeof(TerminateImGuiTask);
        }
    };

    ImGuiRendererSystem& ImGuiRendererSystem::create()
    {
        Super::create();
        engine_instance->thread(ThreadType::RenderThread)->insert_new_task<InitImGuiTask>();
        System::new_system<RenderSystem>()->register_subsystem(this);

        return *this;
    }

    ImGuiRendererSystem& ImGuiRendererSystem::update(float dt)
    {
        Window::instance()->rhi_bind();
        ImGuiRenderer::new_frame();
        Super::update(dt);
        ImGuiRenderer::render();
        return *this;
    }

    ImGuiRendererSystem& ImGuiRendererSystem::wait()
    {
        Super::wait();
        return *this;
    }

    ImGuiRendererSystem& ImGuiRendererSystem::shutdown()
    {
        Super::shutdown();

        engine_instance->thread(ThreadType::RenderThread)->insert_new_task<TerminateImGuiTask>();
        return *this;
    }

    implement_engine_class_default_init(ImGuiRendererSystem);
}// namespace Engine
