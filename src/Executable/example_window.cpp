#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Event/event_data.hpp>
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
            EventSystem::new_system<EventSystem>()->add_listener(EventType::KeyDown, [](const Event& event) {
                const KeyEvent& event_data = event.get<const KeyEvent&>();
                if (event_data.key == Keyboard::Key::N)
                {
                    WindowManager::instance()->create_window(global_window_config, nullptr);
                }
            });


            pipeline                  = new Pipeline();
            pipeline->vertex_shader   = new VertexShader();
            pipeline->fragment_shader = new FragmentShader();

            pipeline->vertex_shader->text_code   = read_buffer("shaders/example_window/vertex.vert");
            pipeline->fragment_shader->text_code = read_buffer("shaders/example_window/fragment.frag");

            pipeline->vertex_shader->init_resource();
            pipeline->fragment_shader->init_resource();

            pipeline->rasterizer.cull_mode = CullMode::None;
            pipeline->depth_test.enable    = false;
            pipeline->render_pass          = WindowManager::instance()->main_window()->render_pass;
            pipeline->color_blending.blend_attachment.emplace_back();

            pipeline->init_resource();
        }

        TestRenderingClient& render(RenderViewport* viewport) override
        {
            viewport->window()->rhi_bind();
            pipeline->rhi_bind();
            engine_instance->rhi()->draw(3);
            return *this;
        }
    };


    implement_engine_class_default_init(TestRenderingClient);
}// namespace Engine
