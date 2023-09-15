#include <Core/benchmark.hpp>
#include <Core/class.hpp>
#include <Core/class_members.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/etl/average.hpp>
#include <Core/file_manager.hpp>
#include <Core/game_controller.hpp>
#include <Core/json.hpp>
#include <Core/package.hpp>
#include <Core/shader_compiler.hpp>
#include <Core/stacktrace.hpp>
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
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Systems/game_controller_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Window/config.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <imgui.h>

#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>

#define OBJECTS_PER_AXIS 2

namespace Engine
{
    void update_camera(Camera* camera, float dt);
    enum class UpdateType
    {
        None,
        Static,
        Dynamic,
    };

    static inline String object_name(Object* object)
    {
        return Strings::format("{} [{}]", object->name(), object->class_instance()->name());
    }

    static String full_class_name(const Class* _class)
    {
        if (!_class)
            return "";
        String name = full_class_name(_class->parent());

        if (name.empty())
        {
            return _class->name();
        }

        return name + " -> " + _class->name();
    }

    static void render_package_tree(Package* package)
    {
        if (ImGui::TreeNode(object_name(package).c_str()))
        {
            ImGui::Text("Inheritance: %s", full_class_name(package->class_instance()).c_str());
            ImGui::NewLine();
            for (Object* object : package->objects())
            {
                Package* p = object->instance_cast<Package>();
                if (p)
                {
                    render_package_tree(p);
                }
                else
                {
                    String name = object->class_instance() ? object->class_instance()->name().c_str() : "nill";
                    ImGui::Text("%s [%s]", object->name().c_str(), name.c_str());
                }
            }

            ImGui::TreePop();
        }
    }

    class GameSystem : public Singletone<GameSystem, System>
    {
        declare_class(GameSystem, System);

    public:
        Package* package           = nullptr;
        StaticMeshComponent* mesh1 = nullptr;
        StaticMeshComponent* mesh2 = nullptr;
        Shader* shader             = nullptr;
        Shader* framebuffer_shader = nullptr;
        Camera* camera             = nullptr;
        Texture2D* texture         = nullptr;
        UniformStruct camera_ubo;
        UniformStruct ubo;
        Vector<UniformStructInstance*> ubo_struct_instance;
        UniformStruct fragment_ubo;
        UniformStructInstance* camera_ubo_buffer;
        ScriptFunction script_function;


        Average<double> fps;
        Average<double> fps_by_time;


        UpdateType type = UpdateType::Static;
        static GameSystem* _M_instance;


    public:
        GameSystem& shutdown() override
        {
            Super::shutdown();
            engine_instance->renderer()->wait_idle();
            return *this;
        }

        GameSystem& create() override
        {
            EngineSystem::instance()->add_object(this, true);
            package = Object::load_package("TestResources");
            if (package == nullptr)
                throw EngineException("Cannot load resources!");

            mesh1  = package->find_object_checked<StaticMeshComponent>("Cube");
            mesh2  = package->find_object_checked<StaticMeshComponent>("Mesh 2");
            shader = mesh2->material_applier(0)->shader();

            info_log("GameInit", "Material name: %s", mesh2->lods[0].material_reference.instance()->name().c_str());

            framebuffer_shader = mesh1->material_applier(0)->shader();
            camera             = package->find_object_checked<Camera>("Camera");
            camera->load();
            camera->aspect(engine_instance->window()->width() / engine_instance->window()->height());

            texture = package->find_object_checked<Texture2D>("Trinex Texture");

            camera_ubo.add_field(DynamicStructField::field_of<Matrix4f>("field1"));
            camera_ubo_buffer = camera_ubo.create_instance();

            ubo.add_field(DynamicStructField::field_of<Matrix4f>("field1"));


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


            fragment_ubo.add_field(DynamicStructField::field_of<Vector3D>("field"));

            camera->ready();


            FileReader* reader =
                    new FileReader(FS::path("/home/programier/Projects/C++/TrinexEngine/scripts/script.cpp"));
            String code;
            code.resize(reader->size());
            reader->read((byte*) code.data(), code.size());
            code.push_back('\0');
            delete reader;

            auto module = ScriptEngine::instance()->module("test");
            module.add_script_section("code", code.c_str());
            module.build();
            script_function = module.function_by_name("script_function");


            return *this;
        }

        GameSystem& update(float dt) override
        {
            static int i = 0;
            i++;

            if (i % 20 == 0)
            {
                script_function.prepare().call();
                logger->log("Script", "Script result: %f", script_function.result_float());
                script_function.unbind_context();
            }

            Super::update(dt);
            VertexBuffer& vertex_buffer        = mesh1->lods[0].vertex_buffer;
            IndexBuffer& index_buffer          = mesh1->lods[0].index_buffer;
            VertexBuffer& output_vertex_buffer = mesh2->lods[0].vertex_buffer;
            IndexBuffer& output_index_buffer   = mesh2->lods[0].index_buffer;


            engine_instance->renderer()->begin();

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
                                _x = float(x) * 5 + glm::sin(engine_instance->time_seconds() + float(x + y + z));
                                _y = float(y) * 5 + glm::cos(engine_instance->time_seconds() + float(x + y + z));
                                _z = float(z) * 5 + glm::sin(engine_instance->time_seconds() + float(x + y + z));
                            }

                            instance->get_ref<Matrix4f>(0) = glm::translate(
                                    glm::rotate(Constants::identity_matrix, glm::radians(90.f), Constants::OX),
                                    Vector3D(_x, _y, _z));
                        }

                        vertex_buffer.bind();
                        camera_ubo_buffer->bind(0);
                        instance->bind(1);
                        texture->bind(2);
                        engine_instance->renderer()->draw_indexed(index_buffer.elements_count(), 0);
                    }
                }
            }


            engine_instance->window()->bind();
            shader->use();
            output_vertex_buffer.bind();
            output_index_buffer.bind();
            GBuffer::instance()->buffer_data().albedo.ptr()->bind();
            GBuffer::instance()->previous_buffer_data().albedo.ptr()->bind(1);
            engine_instance->renderer()->draw_indexed(output_index_buffer.elements_count(), 0);

            {

                ImGuiRenderer::new_frame();

                ImGui::Begin("TrinexEngine");

                fps.push(1.0 / dt);
                ImGui::Text("API: %s", engine_config.api.c_str());
                ImGui::Text("FPS: %lf", fps.average());
                {
                    const Transform& transform = camera->transform;
                    ImGui::Text("Pos: X = %f, Y = %f, Z = %f", transform.position().x, transform.position().y,
                                transform.position().z);
                    ImGui::Text("Memory usage: %zu bytes [%zu KB]", MemoryManager::allocated_size(),
                                MemoryManager::allocated_size() / 1024);


                    Size2D size = Monitor::size();

                    ImGui::Text("Monitor size: %f x %f", size.x, size.y);
                    size = Monitor::physical_size(PhysicalSizeMetric::Сentimeters);
                    ImGui::Text("Physical monitor size: %f x %f", size.x, size.y);
                    ImGui::Text("TEST: %p", Object::find_object("Camera"));


                    ImGui::NewLine();

                    render_package_tree(Object::root_package());
                }

                ImGui::End();

                ImGuiRenderer::render();
            }

            if (fps.count() == 60)
            {
                fps.reset();
                fps_by_time.reset();
            }

            engine_instance->renderer()->end();
            engine_instance->window()->swap_buffers();

            camera->update(dt);
            update_camera(camera, dt);

            if (KeyboardSystem::instance()->is_just_pressed(Keyboard::G))
            {
                package->save();
            }

            //            if (KeyboardEvent::just_pressed(Key::F))
            //            {
            //                engine_instance->window()->attribute(
            //                        WindowAttribute::WinFullScreenDesktop,
            //                        !engine_instance->window()->attribute(WindowAttribute::WinFullScreenDesktop));
            //            }

            //            if (KeyboardEvent::just_pressed(Key::Num0))
            //            {
            //                info_log("KEY", "0");
            //                type = UpdateType::None;
            //            }
            //            else if (KeyboardEvent::just_pressed(Key::Num1))
            //            {
            //                info_log("KEY", "1");
            //                type = UpdateType::Static;
            //            }
            //            else if (KeyboardEvent::just_pressed(Key::Num2))
            //            {
            //                info_log("KEY", "2");
            //                type = UpdateType::Dynamic;
            //            }
            //            else if (KeyboardEvent::just_pressed(Key::Num4))
            //            {}

            return *this;
        }
    };


    void print_json(const JSON::Object& object, int tabs = 0)
    {
        for (auto& ell : object)
        {
            String t(tabs, '\t');
            info_log("JSON", "%s %s", t.c_str(), ell.first.c_str());
        }
    }

    int_t GameInit::execute(int_t argc, char** argv)
    {
        //        Events::on_quit.push(on_close);
        //        Events::on_terminate.push(on_terminate);
        //        Events::on_pause.push(on_pause);
        //        Events::on_resume.push(on_resume);

        engine_instance->create_window();

        ImGuiRenderer::init();
        ImGui::GetIO().FontGlobalScale = glm::round(100.f * (Monitor::dpi().ddpi / 141.f)) / 100.f;

        if (engine_instance->system_type() == SystemName::AndroidOS)
        {
            ImGui::GetStyle().ScrollbarSize = 25.f;
        }

        EventSystem::init_all();
        System::new_system<GameSystem>();

        return engine_instance->launch_systems();
    }

    GameSystem* GameSystem::_M_instance = nullptr;

    implement_class(GameInit, "");
    implement_class(GameSystem, "Engine");
    implement_default_initialize_class(GameInit);
    implement_default_initialize_class(GameSystem);

}// namespace Engine


static void preinit()
{
    Engine::EngineInstance::project_name("TrinexEngineLauncher");
}

static Engine::PreInitializeController controller(preinit);
