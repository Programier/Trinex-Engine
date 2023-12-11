#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <Systems/event_system.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>


#include <Core/file_manager.hpp>
#include <Graphics/pipeline.hpp>

namespace Engine
{

    static bool registered = false;

    Buffer read_buffer(const Path& filename, bool is_text = false)
    {
        FileReader reader = FileManager::root_file_manager()->work_dir() / filename;

        if (reader.is_open())
        {
            Buffer buffer(reader.size(), 0);
            reader.read(buffer.data(), buffer.size());
            if (is_text)
                buffer.push_back(0);
            return buffer;
        }

        return {};
    }

    class TestRenderingClient : public ViewportClient
    {
        declare_class(TestRenderingClient, ViewportClient);


        Pipeline* pipeline;

    public:
        TestRenderingClient()
        {
            if (!registered)
            {
                EventSystem::new_system<EventSystem>()->add_listener(EventType::KeyDown, [](const Event& event) {
                    const KeyEvent& event_data = event.get<const KeyEvent&>();
                    if (event_data.key == Keyboard::Key::N)
                    {
                        global_window_config.vsync = false;
                        WindowManager::instance()->create_window(global_window_config, nullptr);
                    }
                });

                registered = true;
            }


            pipeline                  = new Pipeline();
            pipeline->vertex_shader   = new VertexShader();
            pipeline->fragment_shader = new FragmentShader();

            if (engine_instance->api() == EngineAPI::Vulkan)
            {
                pipeline->vertex_shader->binary_code   = read_buffer("shaders/example_window/vertex.vm", false);
                pipeline->fragment_shader->binary_code = read_buffer("shaders/example_window/fragment.fm", false);
            }
            else
            {
                pipeline->vertex_shader->text_code   = read_buffer("shaders/example_window/vertex.vert", true);
                pipeline->fragment_shader->text_code = read_buffer("shaders/example_window/fragment.frag", true);
            }

            pipeline->vertex_shader->init_resource();
            pipeline->fragment_shader->init_resource();

            pipeline->rasterizer.cull_mode = CullMode::None;
            pipeline->depth_test.enable    = false;
            pipeline->render_pass          = WindowManager::instance()->main_window()->render_pass;
            pipeline->color_blending.blend_attachment.emplace_back();

            pipeline->init_resource();
        }

        TestRenderingClient& on_bind_to_viewport(RenderViewport* viewport) override
        {
            viewport->window()->imgui_initialize();
            return *this;
        }

        TestRenderingClient& prepare_render(RenderViewport* viewport) override
        {
            viewport->window()->imgui_window()->prepare_render();
            return *this;
        }

        TestRenderingClient& update(RenderViewport* viewport, float dt) override
        {

            viewport->window()->imgui_window()->new_frame();
            //draw_dock_space();

            ImGui::Begin("Hello");

            for(auto& [name, instance] : Class::class_table())
            {
                ImGui::Text("%s", name.c_str());
            }

            ImGui::End();
            viewport->window()->imgui_window()->end_frame();
            return *this;
        }

        TestRenderingClient& draw_dock_space()
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);

            ImGui::Begin("DockSpace Demo");

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
            }

            ImGui::End();

            return *this;
        }

        TestRenderingClient& render(RenderViewport* viewport) override
        {
            viewport->window()->rhi_bind();
            pipeline->rhi_bind();
            engine_instance->rhi()->draw(3);


            viewport->window()->imgui_window()->render();

            return *this;
        }

        ~TestRenderingClient()
        {}
    };


    implement_engine_class_default_init(TestRenderingClient);
}// namespace Engine
