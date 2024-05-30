#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/engine_loop.hpp>
#include <Core/export.hpp>
#include <Core/thread.hpp>
#include <android_native_app_glue.h>
#include <android_platform.hpp>

#include <Core/class.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>


class ENGINE_EXPORT AndroidClient : public Engine::ViewportClient
{
    declare_class(AndroidClient, Engine::ViewportClient);

public:
    AndroidClient()
    {}

    ViewportClient& on_bind_viewport(class Engine::RenderViewport* viewport) override
    {
        viewport->window()->imgui_initialize();

        return *this;
    }

    ViewportClient& render(class Engine::RenderViewport* viewport) override
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    ViewportClient& update(class Engine::RenderViewport* viewport, float dt) override
    {
        viewport->window()->imgui_window()->new_frame();
        ImGui::ShowDemoWindow();
        viewport->window()->imgui_window()->end_frame();
        return *this;
    }
};

implement_class_default_init(AndroidClient, );


// This method will be called from android_main or
FORCE_ENGINE_EXPORT extern "C" int trinex_engine_android_main(int argc, const char** argv)
try
{
    Engine::EngineLoop loop;

    int init_status = loop.init(argc, argv);
    auto engine     = Engine::engine_instance;


    Engine::WindowManager::instance()->create_client(Engine::WindowManager::instance()->main_window(), "AndroidClient");

    if (init_status == 0)
    {
        while (!engine->is_requesting_exit())
        {
            loop.update();
        }
    }

    loop.terminate();
    return init_status;
}
catch (std::exception& e)
{
    printf("Exception: %s\n", e.what());
    return 1;
}

// This method will be called from NativeActivity
FORCE_ENGINE_EXPORT extern "C" void android_main(struct android_app* app)
{
    Engine::Platform::initialize_android_application(app);
    trinex_engine_android_main(0, nullptr);
}
