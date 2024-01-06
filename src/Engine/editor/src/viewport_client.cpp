#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/localization.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/thread.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/scene_render_targets.hpp>
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

        _M_package_tree.init(viewport);
        _M_content_browser.init(viewport);
        _M_properties.init(viewport);
        _M_scene_tree.init(viewport);

        _M_package_tree.on_package_select.push(std::bind(&EditorViewportClient::on_package_select, this, std::placeholders::_1));
        _M_content_browser.on_object_selected.push(
                std::bind(&EditorViewportClient::on_object_select, this, std::placeholders::_1));
        _M_sampler = Package::load_package("Editor")->find_object_checked<Sampler>("DefaultSampler");
        return init_world();
    }

    void EditorViewportClient::on_package_select(Package* package)
    {
        _M_content_browser.package = package;
    }

    void EditorViewportClient::on_object_select(Object* object)
    {
        _M_properties.object = object;
    }

    size_t count = 0;

    ViewportClient& EditorViewportClient::render(class RenderViewport* viewport)
    {

        // Render base frame
        GBuffer::instance()->rhi_bind();
        SceneColorOutput::instance()->rhi_bind();
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
            if (ImGui::BeginMenu("editor/View"_localized))
            {
                if (ImGui::MenuItem("editor/Open Material Editor"_localized))
                {
                    open_material_editor();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("editor/Edit"_localized))
            {
                if (ImGui::MenuItem("editor/Reload localization"_localized))
                {
                    Localization::instance()->reload();
                }

                if (ImGui::BeginMenu("editor/Change language"_localized))
                {
                    for (const String& lang : engine_config.languages)
                    {
                        const char* localized = Object::localize("editor/" + lang).c_str();
                        if(ImGui::MenuItem(localized))
                        {
                            Object::language(lang);
                            break;
                        }
                    }

                    ImGui::EndMenu();
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

            auto dock_id_left  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id);
            auto dock_id_ld    = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.25f, nullptr, &dock_id_left);
            auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);
            auto dock_id_down  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.25f, nullptr, &dock_id);

            ImGui::DockBuilderDockWindow(_M_package_tree.name(), dock_id_ld);
            ImGui::DockBuilderDockWindow(_M_scene_tree.name(), dock_id_left);
            ImGui::DockBuilderDockWindow(_M_content_browser.name(), dock_id_down);
            ImGui::DockBuilderDockWindow(_M_properties.name(), dock_id_right);
            ImGui::DockBuilderDockWindow(Object::localize("editor/Viewport Title").c_str(), dock_id);

            ImGui::DockBuilderFinish(dock_id);
        }
    }

    ViewportClient& EditorViewportClient::update(class RenderViewport* viewport, float dt)
    {
        ImGuiRenderer::Window* window = viewport->window()->imgui_window();
        window->new_frame();
        make_dock_window("EditorDock", ImGuiWindowFlags_MenuBar, &EditorViewportClient::render_dock_window, this, viewport);

        _M_package_tree.render(viewport);
        _M_content_browser.render(viewport);
        _M_properties.render(viewport);
        _M_scene_tree.render(viewport);

        create_log_window(dt);
        create_viewport_window(dt);

        _M_script_object.update(dt);

        window->end_frame();

        ++_M_frame;

        return *this;
    }

    EditorViewportClient& EditorViewportClient::init_world()
    {
        World* world                 = World::global();
        _M_scene_tree.root_component = world->scene()->root_component();

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
        if (!ImGui::Begin(Object::localize("editor/Viewport Title").c_str(), nullptr))
        {
            ImGui::End();
            return *this;
        };

        auto* frame = SceneColorOutput::instance()->current_frame();

        if (frame)
        {
            Texture* texture = frame->texture();
            if (texture)
            {
                void* output = ImGuiRenderer::Window::current()->create_texture(texture, _M_sampler)->handle();
                auto size    = ImGui::GetContentRegionAvail();
                ImGui::Image(output, size);
            }
        }

        ImGui::End();
        return *this;
    }
}// namespace Engine
