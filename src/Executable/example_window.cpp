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
        ImGuiRenderer::DrawData draw_data;

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
            ImGui::SetCurrentContext(viewport->window()->imgui_context());
            draw_data.copy(ImGui::GetDrawData());
            return *this;
        }

        TestRenderingClient& update(RenderViewport* viewport, float dt) override
        {

            viewport->window()->imgui_new_frame();

            ImGui::Begin("Hello");
            ImGui::Text("HELLO WORLD");
            ImGui::End();

            viewport->window()->imgui_end_frame();


            //            ImGui::Begin("Hello2");
            //            ImGui::Text("HELLO WORLD");
            //            ImGui::End();

            return *this;
        }

        TestRenderingClient& render(RenderViewport* viewport) override
        {
            viewport->window()->rhi_bind();
            pipeline->rhi_bind();
            engine_instance->rhi()->draw(3);
            viewport->window()->imgui_render(draw_data.draw_data());

            return *this;
        }

        ~TestRenderingClient()
        {}
    };


    implement_engine_class_default_init(TestRenderingClient);
}// namespace Engine
