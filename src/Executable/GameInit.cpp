#include <Core/class.hpp>
#include <Core/config.hpp>
#include <Core/etl/average.hpp>
#include <Core/file_manager.hpp>
#include <Core/package.hpp>
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
#include <thread>

namespace Engine
{

    struct ModelUBO {
        Matrix4f model;
    };

    struct CameraUBO {
        Matrix4f projview;
    };

    struct Kek {
        Vector3D axis;
    };

    static float min_time     = 100;
    static float max_time     = 0;
    static float current_diff = 0;

    void GameInit::loop()
    {
        Window window;

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

        UniformBuffer<CameraUBO> camera_ubo[2];

        Camera* camera = Object::new_instance<Camera>(Vector3D{0, 0, 0.3});
        Camera* model  = Object::new_instance<Camera>(Vector3D{0, 0, 0.0});

        camera->min_render_distance(0.01).max_render_distance(1000.f);
        camera->viewing_angle(glm::radians(70.f));
        camera->aspect(window.width() / window.height());

        for (int i = 0; i < 2; i++)
        {
            camera_ubo[i].buffer.projview = camera->projview();
            camera_ubo[i].create();
        }


        UniformBuffer<ModelUBO> ubo;

        ubo.buffer.model = glm::translate(glm::rotate(Constants::identity_matrix, glm::radians(90.f), Constants::OX),
                                          Vector3D(0, 0, 0.0));
        ubo.create();

        UniformBuffer<Kek> fragment_ubo;
        fragment_ubo.create();

        Average<double> fps;
        static size_t index = 0;


        auto camera_update = Lua::Interpretter::execute_string("return require('camera').update;").get<Lua::function>();


        Average<float> script_average;

        while (window.is_open())
        {
            camera->update();

            script_average.push(camera_update(camera).get<float>());

            if (MouseEvent::scroll_offset().y != 0)
            {
                float current = texture.anisotropic_filtering() + MouseEvent::scroll_offset().y;
                info_log("Antialiazing: %f", current);
                texture.anisotropic_filtering(current);
            }


            _M_renderer->begin();

            camera_ubo[index].buffer.projview = camera->projview();
            camera_ubo[index].update(0, sizeof(CameraUBO));
            fragment_ubo.buffer.axis = camera->transform.front_vector();
            fragment_ubo.update(0, sizeof(Kek));


            ubo.buffer.model = model->transform.matrix();
            ubo.update(0, sizeof(ModelUBO));

            GBuffer::instance()->bind();
            framebuffer_shader->use();
            index_buffer.bind();


            vertex_buffer.bind();
            camera_ubo[index].bind_buffer(0);
            ubo.bind_buffer(1);
            texture.bind(2);

            _M_renderer->draw_indexed(index_buffer.elements_count(), 0);


            window.bind();
            shader->use();
            output_vertex_buffer.bind();
            output_index_buffer.bind();
            GBuffer::instance()->buffer_data().albedo.ptr()->bind();
            GBuffer::instance()->previous_buffer_data().albedo.ptr()->bind(1);

            _M_renderer->draw_indexed(output_index_buffer.elements_count(), 0);


            ImGuiRenderer::new_frame();

            ImGui::Begin("TrinexEngine");

            fps.push(1.0 / Event::diff_time());
            ImGui::Text("API: %s", engine_config.api.c_str());
            ImGui::Text("FPS: %lf", fps.average());
            {
                const Transform& transform = camera->transform;
                ImGui::Text("Pos: X = %f, Y = %f, Z = %f", transform.position().x, transform.position().y,
                            transform.position().z);
                ImGui::Text("Min: %f, Max: %f, Current: %f", min_time, max_time, current_diff);
                ImGui::Text("Script time: %f ms", script_average.average());
            }

            if (fps.count() == 60)
            {
                fps.reset();
                script_average.reset();
            }

            ImGui::End();

            ImGuiRenderer::render();

            _M_renderer->end();


            Event::poll_events();
            window.swap_buffers();
            //update_camera(current_camera);

            if (KeyboardEvent::just_pressed(Key::G))
            {

                package->save();
            }
        }

        _M_renderer->wait_idle();
    }

    int GameInit::execute(int argc, char** argv)
    {
        _M_renderer = Engine::EngineInstance::instance()->renderer();
        Window window;
        window.init({1280, 720}, "Trinex Engine", WindowAttrib::WinResizable);
        window.vsync(true);
        ImGuiRenderer::init();

        loop();
        return 0;
    }

    register_class(GameInit);

}// namespace Engine
