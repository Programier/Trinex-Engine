#include <Core/benchmark.hpp>
#include <Core/class.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/etl/average.hpp>
#include <Core/etl/unique_per_frame_var.hpp>
#include <Core/file_manager.hpp>
#include <Core/package.hpp>
#include <Core/thread.hpp>
#include <GameInitCommandLet.hpp>
#include <Graphics/camera.hpp>
#include <Graphics/framebuffer.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh_component.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/uniform_buffer.hpp>
#include <ImGui/imgui.h>
#include <Window/window.hpp>

#if PLATFORM_ANDROID
#include <Window/monitor.hpp>
#endif

namespace Engine
{

    class Test : public ExecutableObject
    {
    public:
        int_t execute()
        {
            {
                logger->log("Thread name", "%s", Thread::this_thread()->name().c_str());
            }
            return 0;
        }

        ~Test()
        {}
    };


#define OBJECTS_PER_AXIS 10

    void GameInit::loop()
    {
        Test test;

        Thread thread;

        Package* package = Object::load_package("TestResources");
        if (package == nullptr)
            return;
        StaticMeshComponent* mesh1 = package->find_object_checked<StaticMeshComponent>("Cube");
        StaticMeshComponent* mesh2 = package->find_object_checked<StaticMeshComponent>("Mesh 2");

        Shader* shader             = mesh2->material_applier(0)->shader();
        Shader* framebuffer_shader = mesh1->material_applier(0)->shader();

        VertexBuffer& vertex_buffer        = mesh1->lods[0].vertex_buffer;
        IndexBuffer& index_buffer          = mesh1->lods[0].index_buffer;
        VertexBuffer& output_vertex_buffer = mesh2->lods[0].vertex_buffer;
        IndexBuffer& output_index_buffer   = mesh2->lods[0].index_buffer;
        Texture2D& texture                 = *package->find_object_checked<Texture2D>("Trinex Texture");
        Camera* camera                     = package->find_object_checked<Camera>("Camera");
        camera->load();
        camera->aspect(Window::window->width() / Window::window->height());


        UniformStruct camera_ubo;
        camera_ubo.add_field(DynamicStructField::field_of<Matrix4f>());

        UniformStructInstance* camera_ubo_buffer = camera_ubo.create_instance();

        UniformStruct ubo;
        ubo.add_field(DynamicStructField::field_of<Matrix4f>());


        Vector<UniformStructInstance*> ubo_struct_instance;

        for (int x = 0; x < OBJECTS_PER_AXIS; x++)
        {
            for (int y = 0; y < OBJECTS_PER_AXIS; y++)
            {
                for (int z = 0; z < OBJECTS_PER_AXIS; z++)
                {
                    ubo_struct_instance.push_back(ubo.create_instance());
                }
            }
        }


        UniformStruct fragment_ubo;
        fragment_ubo.add_field(DynamicStructField::field_of<Vector3D>());

        UniformStructInstance* fragment_ubo_inst = fragment_ubo.create_instance();


        Average<double> fps;


#define TIMES_COUNT 4
        float times[TIMES_COUNT];

        while (Window::window->is_open())
        {
            {
                BenchMark<std::chrono::milliseconds> bench;
                if (MouseEvent::scroll_offset().y != 0)
                {
                    float current = texture.anisotropic_filtering() + MouseEvent::scroll_offset().y;
                    info_log("Game", "Antialiazing: %f", current);
                    texture.anisotropic_filtering(current);
                }


                _M_renderer->begin();

                camera_ubo_buffer->get_ref<Matrix4f>(0) = camera->projview();
                fragment_ubo_inst->get_ref<Vector3D>(0) = camera->transform.front_vector();

                GBuffer::instance()->bind();
                framebuffer_shader->use();
                index_buffer.bind();

                bench.log_status(false);
                times[0] = bench.time();
            }


            {
                static int updates = OBJECTS_PER_AXIS * OBJECTS_PER_AXIS * OBJECTS_PER_AXIS * 3;

                times[1] = 0;
                for (int x = 0; x < OBJECTS_PER_AXIS; x++)
                {
                    for (int y = 0; y < OBJECTS_PER_AXIS; y++)
                    {
                        for (int z = 0; z < OBJECTS_PER_AXIS; z++)
                        {

                            UniformStructInstance* instance =
                                    ubo_struct_instance[x * (OBJECTS_PER_AXIS * OBJECTS_PER_AXIS) +
                                                        y * OBJECTS_PER_AXIS + z];
                            if (updates > 0)
                            {
                                float _x = float(x) * 5 /*+ glm::sin(Event::time() + float(x + y + z))*/;
                                float _y = float(y) * 5 /*+ glm::cos(Event::time() + float(x + y + z))*/;
                                float _z = float(z) * 5 /*+ glm::sin(Event::time() + float(x + y + z))*/;

                                instance->get_ref<Matrix4f>(0) = glm::translate(
                                        glm::rotate(Constants::identity_matrix, glm::radians(90.f), Constants::OX),
                                        Vector3D(_x, _y, _z));
                                updates--;
                            }

                            {
                                BenchMark<std::chrono::microseconds> bench;
                                vertex_buffer.bind();
                                camera_ubo_buffer->bind(0);
                                instance->bind(1);
                                texture.bind(2);
                                bench.log_status(false);
                                times[1] += bench.time();
                            }


                            {
                                BenchMark<std::chrono::microseconds> bench;
                                _M_renderer->draw_indexed(index_buffer.elements_count(), 0);
                                bench.log_status(false);
                                times[2] += bench.time();
                            }
                        }
                    }
                }
            }

            times[1] /= 1000;
            times[2] /= 1000;


            {
                Window::window->bind();
                shader->use();
                output_vertex_buffer.bind();
                output_index_buffer.bind();
                GBuffer::instance()->buffer_data().albedo.ptr()->bind();
                GBuffer::instance()->previous_buffer_data().albedo.ptr()->bind(1);
                _M_renderer->draw_indexed(output_index_buffer.elements_count(), 0);
            }


            ImGuiRenderer::new_frame();

            ImGui::Begin("TrinexEngine");

            fps.push(1.0 / Event::diff_time());
            ImGui::Text("API: %s", engine_config.api.c_str());
            ImGui::Text("FPS: %lf", fps.average());
            {
                const Transform& transform = camera->transform;
                ImGui::Text("Pos: X = %f, Y = %f, Z = %f", transform.position().x, transform.position().y,
                            transform.position().z);
                ImGui::Text("Script time: %f", camera->script.on_update.last_result().get<float>());
                ImGui::Text("Memory usage: %zu bytes [%zu KB]", MemoryManager::allocated_size(),
                            +MemoryManager::allocated_size() / 1024);

                float sum = 0;
                for (int i = 0; i < TIMES_COUNT; i++)
                {
                    sum += times[i];
                    ImGui::Text("Time[%d] = %f", i, times[i]);
                }

                ImGui::Text("Time[sum] = %f", sum);
            }

            if (fps.count() == 60)
            {
                fps.reset();
            }

            ImGui::End();

            ImGuiRenderer::render();

            {
                BenchMark<std::chrono::milliseconds> bench;
                _M_renderer->end();
                bench.log_status(false);
                times[3] = bench.time();
            }


            Event::poll_events();
            Window::window->swap_buffers();
            camera->update();

            if (KeyboardEvent::just_pressed(Key::G))
            {
                engine_config.save_config("test.conf");
            }
        }

        _M_renderer->wait_idle();
    }

    int GameInit::execute(int argc, char** argv)
    {
        _M_renderer = Engine::EngineInstance::instance()->renderer();
        Window::create_instance();
#if PLATFORM_ANDROID
        Window::window->set_orientation(WindowOrientation::WinOrientationLandscape);
        Window::window->init(Monitor::size().y, Monitor::size().x, "Trinex Engine", WindowAttrib::WinFullScreenDesktop);
#else
        Window::window->init({1280, 720}, "Trinex Engine", WindowAttrib::WinResizable);
#endif
        Window::window->initialize_api();
        Window::window->vsync(false);
        ImGuiRenderer::init();

        loop();
        return 0;
    }

    register_class(GameInit);

}// namespace Engine


static void preinit()
{
    Engine::EngineInstance::project_name("TrinexEngineLauncher");
}

static Engine::PreInitializeController controller(preinit);
