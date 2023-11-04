#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/imgui.hpp>
#include <Platform/thread.hpp>
#include <Systems/imgui_system.hpp>
#include <Systems/render_system.hpp>
#include <Window/window.hpp>

namespace Engine
{

    class InitImGuiTask : public SingleTimeExecutableObject
    {
    public:
        int_t execute() override
        {
            ImGuiRenderer::init();
            return 0;
        }
    };


    class TerminateImGuiTask : public SingleTimeExecutableObject
    {
    public:
        int_t execute() override
        {
            ImGuiRenderer::terminate();
            return 0;
        }
    };

    ImGuiRendererSystem& ImGuiRendererSystem::create()
    {
        Super::create();
        engine_instance->thread(ThreadType::RenderThread)->push_task(new InitImGuiTask());
        System::new_system<RenderSystem>()->register_subsystem(this);

        return *this;
    }

    ImGuiRendererSystem& ImGuiRendererSystem::update(float dt)
    {
        Window::instance()->bind();
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

        engine_instance->thread(ThreadType::RenderThread)->push_task(new TerminateImGuiTask());
        return *this;
    }

    implement_engine_class_default_init(ImGuiRendererSystem);
}// namespace Engine
