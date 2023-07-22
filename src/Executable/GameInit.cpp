#include <Core/benchmark.hpp>
#include <Core/class.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/etl/average.hpp>
#include <Core/etl/unique_per_frame_var.hpp>
#include <Core/file_manager.hpp>
#include <Core/package.hpp>
#include <Core/shader_compiler.hpp>
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
#include <Window/monitor.hpp>
#include <Window/window.hpp>


#define OBJECTS_PER_AXIS 2

namespace Engine
{


    class UILogger : public Logger
    {

    private:
        List<String> _M_messages;

    public:
        UILogger& write_log(const char* level, const char* tag, const char* format, va_list list)
        {
            char buffer[1024];
            vsprintf(buffer + sprintf(buffer, "[%s][%s] ", level, tag), format, list);
            _M_messages.push_back(buffer);
            return *this;
        }

        Logger& log(const char* tag, const char* format, ...) override
        {
            va_list args;
            va_start(args, format);
            write_log(__FUNCTION__, tag, format, args);
            va_end(args);
            return *this;
        }

        Logger& debug(const char* tag, const char* format, ...) override
        {
            va_list args;
            va_start(args, format);
            write_log(__FUNCTION__, tag, format, args);
            va_end(args);
            return *this;
        }

        Logger& warning(const char* tag, const char* format, ...) override
        {
            va_list args;
            va_start(args, format);
            write_log(__FUNCTION__, tag, format, args);
            va_end(args);
            return *this;
        }

        Logger& error(const char* tag, const char* format, ...) override
        {
            va_list args;
            va_start(args, format);
            write_log(__FUNCTION__, tag, format, args);
            va_end(args);
            return *this;
        }

        Logger& error(const char* tag, const String& msg, const MessageList& messages) override
        {
            return *this;
        }

        void render()
        {
            for (auto& msg : _M_messages)
            {
                ImGui::Text("%s", msg.c_str());
            }
        }
    };


    static void render_package_tree(Package* package)
    {
        if (ImGui::TreeNode(package->name().c_str()))
        {
            for (auto& pair : package->objects())
            {
                Object* object = pair.second;

                Package* p = object->instance_cast<Package>();
                if (p)
                {
                    render_package_tree(p);
                }
                else
                {
                    const char* name = object->class_instance() ? object->class_instance()->name().c_str() : "Class";
                    ImGui::Text("%s [%s]", object->name().c_str(), name);
                }
            }

            ImGui::TreePop();
        }
    }


    void GameInit::loop()
    {
        Package* package = Object::load_package("TestResources");
        if (package == nullptr)
            return;
        StaticMeshComponent* mesh1 = package->find_object_checked<StaticMeshComponent>("Cube");
        StaticMeshComponent* mesh2 = package->find_object_checked<StaticMeshComponent>("Mesh 2");

        Shader* shader = mesh2->material_applier(0)->shader();
        logger->log("GameInit", "Material name: %s", mesh2->lods[0].material_reference.instance()->name().c_str());
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
        camera_ubo.add_field(DynamicStructField::field_of<Matrix4f>("field1"));

        UniformStructInstance* camera_ubo_buffer = camera_ubo.create_instance();

        UniformStruct ubo;
        ubo.add_field(DynamicStructField::field_of<Matrix4f>("field1"));


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
        fragment_ubo.add_field(DynamicStructField::field_of<Vector3D>("field"));

        Average<double> fps;
        Average<double> fps_by_time;

        enum UpdateType
        {
            None,
            Static,
            Dynamic,
        };

        static UpdateType type = UpdateType::Static;

        while (Window::window->is_open())
        {
            _M_renderer->begin();

            camera_ubo_buffer->get_ref<Matrix4f>(0) = camera->projview();

            GBuffer::instance()->bind();
            framebuffer_shader->use();
            index_buffer.bind();


            for (int x = 0; x < OBJECTS_PER_AXIS; x++)
            {
                for (int y = 0; y < OBJECTS_PER_AXIS; y++)
                {
                    for (int z = 0; z < OBJECTS_PER_AXIS; z++)
                    {

                        UniformStructInstance* instance =
                                ubo_struct_instance[x * (OBJECTS_PER_AXIS * OBJECTS_PER_AXIS) + y * OBJECTS_PER_AXIS +
                                                    z];
                        if (type != UpdateType::None)
                        {
                            float _x, _y, _z;

                            if (type == UpdateType::Static)
                            {
                                _x = float(x) * 5;
                                _y = float(y) * 5;
                                _z = float(z) * 5;
                            }
                            else
                            {
                                _x = float(x) * 5 + glm::sin(Event::time() + float(x + y + z));
                                _y = float(y) * 5 + glm::cos(Event::time() + float(x + y + z));
                                _z = float(z) * 5 + glm::sin(Event::time() + float(x + y + z));
                            }

                            instance->get_ref<Matrix4f>(0) = glm::translate(
                                    glm::rotate(Constants::identity_matrix, glm::radians(90.f), Constants::OX),
                                    Vector3D(_x, _y, _z));
                        }

                        vertex_buffer.bind();
                        camera_ubo_buffer->bind(0);
                        instance->bind(1);
                        texture.bind(2);
                        _M_renderer->draw_indexed(index_buffer.elements_count(), 0);
                    }
                }
            }


            Window::window->bind();
            shader->use();
            output_vertex_buffer.bind();
            output_index_buffer.bind();
            GBuffer::instance()->buffer_data().albedo.ptr()->bind();
            GBuffer::instance()->previous_buffer_data().albedo.ptr()->bind(1);
            _M_renderer->draw_indexed(output_index_buffer.elements_count(), 0);

            {

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
                                MemoryManager::allocated_size() / 1024);


                    Size2D size = Monitor::size();

                    ImGui::Text("Monitor size: %f x %f", size.x, size.y);
                    size = Monitor::physical_size(PhysicalSizeMetric::Сentimeters);
                    ImGui::Text("Physical monitor size: %f x %f", size.x, size.y);
                    ImGui::Text("TEST: %p", Object::find_object("Engine::Camera"));


                    ImGui::NewLine();

                    render_package_tree(Object::root_package());
                }

                ImGui::End();


                ImGui::Begin("Logs");
                reinterpret_cast<UILogger*>(logger)->render();
                ImGui::End();

                ImGuiRenderer::render();
            }

            if (fps.count() == 60)
            {
                fps.reset();
                fps_by_time.reset();
            }

            _M_renderer->end();

            Event::poll_events();
            Window::window->swap_buffers();
            camera->update();

            if (KeyboardEvent::just_pressed(Key::G))
            {
                package->save();
            }

            if (KeyboardEvent::just_pressed(Key::F))
            {
                Window::window->attribute(WinFullScreenDesktop, !Window::window->attribute(WinFullScreenDesktop));
            }

            if (KeyboardEvent::just_pressed(Key::Num0))
            {
                logger->log("KEY", "0");
                type = UpdateType::None;
            }
            else if (KeyboardEvent::just_pressed(Key::Num1))
            {
                logger->log("KEY", "1");
                type = UpdateType::Static;
            }
            else if (KeyboardEvent::just_pressed(Key::Num2))
            {
                logger->log("KEY", "2");
                type = UpdateType::Dynamic;
            }
            else if (KeyboardEvent::just_pressed(Key::Num4))
            {}
        }

        _M_renderer->wait_idle();
    }


    static void on_close()
    {
        logger->log("APP", __PRETTY_FUNCTION__);
    }

    static void on_terminate()
    {
        logger->log("APP", __PRETTY_FUNCTION__);
    }

    static void on_pause()
    {
        logger->log("APP", __PRETTY_FUNCTION__);
    }

    static void on_resume()
    {
        logger->log("APP", __PRETTY_FUNCTION__);
    }


    int_t GameInit::execute(int_t argc, char** argv)
    {
        Event::on_quit.push(on_close);
        Event::on_terminate.push(on_terminate);
        Event::on_pause.push(on_pause);
        Event::on_resume.push(on_resume);

        _M_renderer = Engine::EngineInstance::instance()->renderer();
        Window::create_instance();
#if PLATFORM_ANDROID
        Window::window->set_orientation(WindowOrientation::WinOrientationLandscape);
        Window::window->init(Monitor::size().y, Monitor::size().x, "Trinex Engine", WindowAttrib::WinFullScreenDesktop);
#else
        Window::window->init({1280, 720}, "Trinex Engine", WindowAttrib::WinResizable);
#endif
        Window::window->initialize_api();
        //Window::window->vsync(true);
        ImGuiRenderer::init();


        ImGui::GetIO().FontGlobalScale = glm::round(100.f * (Monitor::dpi().ddpi / 141.f)) / 100.f;

        if (engine_instance->system_type() == SystemName::AndroidOS)
        {
            ImGui::GetStyle().ScrollbarSize = 25.f;
        }
        loop();
        return 0;
    }

    register_class(GameInit);

}// namespace Engine


static void preinit()
{
    static Engine::UILogger new_logger;
    Engine::logger = &new_logger;

    Engine::EngineInstance::project_name("TrinexEngineLauncher");
}

static Engine::PreInitializeController controller(preinit);
