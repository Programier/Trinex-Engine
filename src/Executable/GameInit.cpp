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
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/uniform_buffer.hpp>
#include <ImGui/imgui.h>
#include <Window/window.hpp>

namespace Engine
{

    struct ModelUBO {
        Matrix4f model;
    };

    struct CameraUBO {
        Matrix4f projview;
    };


    static void update_camera(Camera* camera)
    {
        static float speed  = 5.0f;
        float current_speed = speed * Event::diff_time();

        if (KeyboardEvent::pressed(KEY_W))
        {
            camera->move(camera->front_vector() * current_speed);
        }

        if (KeyboardEvent::pressed(KEY_S))
        {
            camera->move(camera->front_vector() * -current_speed);
        }

        if (KeyboardEvent::pressed(KEY_D))
        {
            camera->move(camera->right_vector() * current_speed);
        }

        if (KeyboardEvent::pressed(KEY_A))
        {
            camera->move(camera->right_vector() * -current_speed);
        }

        if (KeyboardEvent::pressed(KEY_SPACE))
        {
            float k = (KeyboardEvent::pressed(KEY_LEFT_SHIFT) ? -1.f : 1.f);
            camera->move(camera->up_vector() * current_speed * k);
        }

        if (KeyboardEvent::just_pressed(KEY_ENTER))
        {
            MouseEvent::relative_mode(!MouseEvent::relative_mode());
        }

        if (MouseEvent::relative_mode())
        {
            auto offset = MouseEvent::offset() / (Window::size());
            camera->rotate(-offset.x, Constants::OY);
            camera->rotate(offset.y, camera->right_vector());
        }
    }

    void GameInit::loop()
    {
        Window window;

        Package* package  = Object::load_package("TestResources");
        StaticMesh* mesh1 = package->find_object_checked<StaticMesh>("Mesh 1");
        StaticMesh* mesh2 = package->find_object_checked<StaticMesh>("Mesh 2");

        Shader* shader                     = package->find_object_checked<Shader>("Output Shader");
        Shader* framebuffer_shader         = package->find_object_checked<Shader>("GBuffer Shader");
        VertexBuffer& vertex_buffer        = *mesh1->lods[0].vertex_buffer;
        IndexBuffer& index_buffer          = *mesh1->lods[0].index_buffer;
        VertexBuffer& output_vertex_buffer = *mesh2->lods[0].vertex_buffer;
        IndexBuffer& output_index_buffer   = *mesh2->lods[0].index_buffer;
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


        Camera* current_camera = camera;


        Average<double> fps;
        static size_t index = 0;
        while (window.is_open())
        {

            if (MouseEvent::scroll_offset().y != 0)
            {
                float current = texture.anisotropic_filtering() + MouseEvent::scroll_offset().y;
                info_log("Antialiazing: %f", current);
                texture.anisotropic_filtering(current);
            }


            _M_renderer->begin();

            camera_ubo[index].buffer.projview = camera->projview();
            camera_ubo[index].update(0, sizeof(CameraUBO));

            ubo.buffer.model = model->model();
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

            _M_renderer->draw_indexed(output_index_buffer.elements_count(), 0);


            ImGuiRenderer::new_frame();

            ImGui::Begin("TrinexEngine");

            fps.push(1.0 / Event::diff_time());
            ImGui::Text("API: %s", engine_config.api.c_str());
            ImGui::Text("FPS: %lf", fps.average());

            if (fps.count() == 60)
            {
                fps.reset();
            }

            ImGui::End();

            ImGuiRenderer::render();

            _M_renderer->end();


            Event::poll_events();
            window.swap_buffers();
            update_camera(current_camera);

            if (KeyboardEvent::just_pressed(KEY_G))
            {

                package->save();
            }


            if (KeyboardEvent::just_pressed(KEY_1))
            {
                current_camera = camera;
            }
            else if (KeyboardEvent::just_pressed(KEY_2))
            {
                current_camera = model;
            }
        }

        engine_config.save("TrinexEngine/configs/config.cfg");
        _M_renderer->wait_idle();
    }

    int GameInit::execute(int argc, char** argv)
    {
        _M_renderer = Engine::EngineInstance::instance()->renderer();
        Window window;
        window.init({1280, 720}, "Trinex Engine", WindowAttrib::WinResizable);
        ImGuiRenderer::init();

        loop();
        return 0;
    }

    register_class(GameInit, Engine::CommandLet);

}// namespace Engine
