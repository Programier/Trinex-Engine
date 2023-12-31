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
#include <imgui_internal.h>
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

        String new_title = window->title() + Strings::format(" [{} RHI]", engine_instance->rhi()->name().c_str());
        window->title(new_title);
        engine_instance->thread(ThreadType::RenderThread)->wait_all();
        _M_script_object = ScriptModule::global().create_script_object("Viewport");
        _M_script_object.on_create(this);
        EventSystem::new_system<EventSystem>()->process_event_method(EventSystem::PoolEvents);

        _M_package_tree.list    = &_M_window_list;
        _M_content_browser.list = &_M_window_list;
        _M_package_tree.on_package_select.push(std::bind(&EditorViewportClient::on_package_select, this, std::placeholders::_1));
        return init_world();
    }

    void EditorViewportClient::on_package_select(Package* package)
    {
        _M_content_browser.package = package;
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

    void EditorViewportClient::render_dock_window(void* userdata)
    {
        auto dock_id                       = ImGui::GetID("EditorDock##Dock");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags);

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

        if (_M_frame == 0)
        {
            ImGui::DockBuilderRemoveNode(dock_id);
            ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);


            auto dock_id_left = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.2f, nullptr, &dock_id);

            ImGui::DockBuilderDockWindow(_M_package_tree.name(), dock_id_left);
            ImGui::DockBuilderDockWindow(_M_content_browser.name(), dock_id_down);
            //ImGui::DockBuilderDockWindow("Viewport", dock_id);

            ImGui::DockBuilderFinish(dock_id);
        }
    }

    ViewportClient& EditorViewportClient::update(class RenderViewport* viewport, float dt)
    {
        ImGuiRenderer::Window* window = viewport->window()->imgui_window();
        window->new_frame();
        make_dock_window("EditorDock", ImGuiWindowFlags_MenuBar, &EditorViewportClient::render_dock_window, this, nullptr);

        create_properties_window(dt);
        create_log_window(dt);
        create_viewport_window(dt);

        _M_package_tree.render(viewport);
        _M_content_browser.render(viewport);
        _M_window_list.render(viewport);

        _M_script_object.update(dt);
        window->end_frame();

        ++_M_frame;

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
        //        if (!ImGui::Begin("Properties"))
        //        {
        //            ImGui::End();
        //            return *this;
        //        }

        //        ImGui::Text("FPS: %f", 1.0 / dt);
        //        ImGui::End();
        return *this;
    }


    EditorViewportClient& EditorViewportClient::create_log_window(float dt)
    {
        //        if (!ImGui::Begin("Logs"))
        //        {
        //            ImGui::End();
        //            return *this;
        //        }

        //        ImGui::End();
        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_viewport_window(float dt)
    {
        //        if (!ImGui::Begin("Viewport", nullptr))
        //        {
        //            ImGui::End();
        //            return *this;
        //        };

        //        //        auto size = ImGui::GetContentRegionAvail();
        //        //        ImGui::Image(_M_imgui_texture->handle(), size);

        //        ImGui::End();

        return *this;
    }
}// namespace Engine
