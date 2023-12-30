#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <ScriptEngine/script_module.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <dock_window.hpp>
#include <theme.hpp>
#include <viewport_client.hpp>

namespace Engine
{
    implement_engine_class_default_init(EditorViewportClient);

    EditorViewportClient::EditorViewportClient()
    {}

    ViewportClient& EditorViewportClient::on_bind_to_viewport(class RenderViewport* viewport)
    {
        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot bind client to non-window viewport!");
        }

        window->imgui_initialize(initialize_theme);

        // Update window name
        String new_title = window->title() + Strings::format(" [{} RHI]", engine_instance->rhi()->name().c_str());
        window->title(new_title);

        engine_instance->thread(ThreadType::RenderThread)->wait_all();

        _M_script_object = ScriptModule::global().create_script_object("Viewport");
        _M_script_object.on_create(this);

        EventSystem::new_system<EventSystem>()->process_event_method(EventSystem::PoolEvents);
        return init_world();
    }

    ViewportClient& EditorViewportClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    ViewportClient& EditorViewportClient::prepare_render(class RenderViewport* viewport)
    {
        viewport->window()->imgui_window()->prepare_render();
        return *this;
    }

    ViewportClient& EditorViewportClient::destroy_script_object(ScriptObject* object)
    {
        if (*object == _M_script_object)
        {
            _M_script_object.remove_reference();
        }
        return *this;
    }


    static void open_material_editor()
    {
        WindowConfig new_config = global_window_config;
        new_config.title        = "Material Editor";
        new_config.client       = "Engine::MaterialEditorClient";
        WindowManager::instance()->create_window(new_config);
    }

    static void create_bar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Open Material Editor"))
                {
                    open_material_editor();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    ViewportClient& EditorViewportClient::update(class RenderViewport* viewport, float dt)
    {
        ImGuiRenderer::Window* window = viewport->window()->imgui_window();
        window->new_frame();
        make_dock_window("EditorDock", create_bar, ImGuiWindowFlags_MenuBar);
        create_scene_tree_window(dt);
        create_properties_window(dt);
        create_log_window(dt);
        create_viewport_window(dt);

        _M_script_object.update(dt);
        window->end_frame();

        return *this;
    }

    EditorViewportClient& EditorViewportClient::init_world()
    {
        //        World* world = Object::find_object_checked<System>("Engine::Systems::EngineSystem")
        //                               ->find_subsystem("LogicSystem::Global World")
        //                               ->instance_cast<World>();
        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_properties_window(float dt)
    {
        if (!ImGui::Begin("Properties"))
        {
            ImGui::End();
            return *this;
        }

        ImGui::Text("FPS: %f", 1.0 / dt);
        ImGui::End();
        return *this;
    }

    static void render_objects_tree(Package* package)
    {
        if (ImGui::TreeNode(package->string_name().c_str()))
        {

            ImGui::Indent(10.f);
            for (auto& [name, object] : package->objects())
            {
                Package* new_package = object->instance_cast<Package>();
                if (new_package)
                {
                    render_objects_tree(new_package);
                }
                else
                {
                    ImGui::Text("%s", object->string_name().c_str());
                }
            }

            ImGui::Unindent(10.f);
            ImGui::TreePop();
        }
    }

    static void render_system_tree(System* system)
    {
        if (ImGui::TreeNode(system->string_name().c_str()))
        {
            ImGui::Indent(10.f);
            for (System* subsystem : system->subsystems())
            {
                render_system_tree(subsystem);
            }

            ImGui::Unindent(10.f);
            ImGui::TreePop();
        }
    }


    EditorViewportClient& EditorViewportClient::create_scene_tree_window(float dt)
    {
        if (!ImGui::Begin("Scene Tree"))
        {
            ImGui::End();
            return *this;
        }

        render_objects_tree(Object::root_package());
        render_system_tree(EngineSystem::instance());
        ImGui::End();

        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_log_window(float dt)
    {
        if (!ImGui::Begin("Logs"))
        {
            ImGui::End();
            return *this;
        }

        ImGui::End();
        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_viewport_window(float dt)
    {
        if (!ImGui::Begin("Viewport", nullptr))
        {
            ImGui::End();
            return *this;
        };

        //        auto size = ImGui::GetContentRegionAvail();
        //        ImGui::Image(_M_imgui_texture->handle(), size);

        ImGui::End();

        return *this;
    }
}// namespace Engine
