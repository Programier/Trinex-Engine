#include <Core/class.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/etl/average.hpp>
#include <Core/etl/unique_per_frame_var.hpp>
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

#if PLATFORM_ANDROID
#include <Window/monitor.hpp>
#endif

namespace Engine
{
    void GameInit::loop()
    {
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


        UniformBuffer camera_ubo;
        camera_ubo.uniform_struct.add_field(DynamicStruct::Field::field_of<Matrix4f>());
        camera_ubo.create();

        UniquePerFrameVariable<DynamicStructInstance*> camera_ubo_buffer;
        camera_ubo_buffer.push_by_func(3, &DynamicStruct::create_instance, camera_ubo.uniform_struct);


        UniformBuffer ubo;
        ubo.uniform_struct.add_field(DynamicStruct::Field::field_of<Matrix4f>());


        DynamicStructInstance* ubo_struct_instance = ubo.uniform_struct.create_instance();
        ubo_struct_instance->get_ref<Matrix4f>(0)  = glm::translate(
                glm::rotate(Constants::identity_matrix, glm::radians(90.f), Constants::OX), Vector3D(0, 0, 0.0));
        ubo.create(ubo_struct_instance);


        UniformBuffer fragment_ubo;
        fragment_ubo.uniform_struct.add_field(DynamicStruct::Field::field_of<Vector3D>());
        fragment_ubo.create();

        UniquePerFrameVariable<DynamicStructInstance*> fragment_ubo_inst;
        fragment_ubo_inst.push_by_func(3, &DynamicStruct::create_instance, fragment_ubo.uniform_struct);

        Average<double> fps;

        while (Window::window->is_open())
        {
            if (MouseEvent::scroll_offset().y != 0)
            {
                float current = texture.anisotropic_filtering() + MouseEvent::scroll_offset().y;
                info_log("Game", "Antialiazing: %f", current);
                texture.anisotropic_filtering(current);
            }


            _M_renderer->begin();

            camera_ubo_buffer.get()->get_ref<Matrix4f>(0) = camera->projview();
            camera_ubo.update(camera_ubo_buffer);

            fragment_ubo_inst.get()->get_ref<Vector3D>(0) = camera->transform.front_vector();
            fragment_ubo.update(fragment_ubo_inst);

            GBuffer::instance()->bind();
            framebuffer_shader->use();
            index_buffer.bind();


            vertex_buffer.bind();
            camera_ubo.bind(0);
            ubo.bind(1);
            texture.bind(2);

            _M_renderer->draw_indexed(index_buffer.elements_count(), 0);


            Window::window->bind();
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
                ImGui::Text("Script time: %f", camera->script.on_update.last_result().get<float>());
            }

            if (fps.count() == 60)
            {
                fps.reset();
            }

            ImGui::End();

            ImGuiRenderer::render();

            _M_renderer->end();


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
        Window::window->vsync(true);
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
