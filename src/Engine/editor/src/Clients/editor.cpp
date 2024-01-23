#include <Clients/editor_client.hpp>
#include <Clients/open_client.hpp>
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

namespace Engine
{
    implement_engine_class_default_init(EditorClient);

    EditorClient::EditorClient()
    {}

    void EditorClient::on_package_tree_close()
    {
        _M_package_tree = nullptr;
    }

    void EditorClient::on_content_browser_close()
    {
        _M_content_browser = nullptr;
    }

    void EditorClient::on_properties_window_close()
    {
        _M_properties = nullptr;
    }

    void EditorClient::on_scene_tree_close()
    {
        _M_scene_tree = nullptr;
    }

    EditorClient& EditorClient::create_package_tree()
    {
        _M_package_tree = ImGuiRenderer::Window::current()->window_list.create<ImGuiPackageTree>();
        _M_package_tree->on_package_select.push(std::bind(&EditorClient::on_package_select, this, std::placeholders::_1));
        _M_package_tree->on_close.push(std::bind(&EditorClient::on_package_tree_close, this));
        return *this;
    }

    EditorClient& EditorClient::create_content_browser()
    {
        _M_content_browser = ImGuiRenderer::Window::current()->window_list.create<ImGuiContentBrowser>();
        _M_content_browser->on_close.push(std::bind(&EditorClient::on_content_browser_close, this));
        _M_content_browser->on_object_selected.push(std::bind(&EditorClient::on_object_select, this, std::placeholders::_1));

        if (_M_package_tree)
        {
            _M_content_browser->package = _M_package_tree->selected_package();
        }
        return *this;
    }

    EditorClient& EditorClient::create_properties_window()
    {
        _M_properties = ImGuiRenderer::Window::current()->window_list.create<ImGuiObjectProperties>();
        _M_properties->on_close.push(std::bind(&EditorClient::on_properties_window_close, this));

        if (_M_content_browser)
        {
            on_object_select(_M_content_browser->selected);
        }
        return *this;
    }

    EditorClient& EditorClient::create_scene_tree()
    {
        _M_scene_tree = ImGuiRenderer::Window::current()->window_list.create<ImGuiSceneTree>();
        _M_scene_tree->on_close.push(std::bind(&EditorClient::on_scene_tree_close, this));

        World* world                  = World::global();
        _M_scene_tree->root_component = world->scene()->root_component();
        return *this;
    }

    ViewportClient& EditorClient::on_bind_to_viewport(class RenderViewport* viewport)
    {
        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot bind client to non-window viewport!");
        }

        window->imgui_initialize(initialize_theme);

        String new_title = Strings::format("Trinex Editor [{} RHI]", engine_instance->rhi()->name().c_str());
        window->title(new_title);
        engine_instance->thread(ThreadType::RenderThread)->wait_all();
        _M_script_object = ScriptModule::global().create_script_object("Viewport");
        _M_script_object.on_create(this);
        EventSystem::new_system<EventSystem>()->process_event_method(EventSystem::PoolEvents);


        ImGuiRenderer::Window* imgui_window = window->imgui_window();
        ImGuiRenderer::Window* prev_window  = ImGuiRenderer::Window::current();
        ImGuiRenderer::Window::make_current(imgui_window);

        create_package_tree();
        create_content_browser();
        create_properties_window();
        create_scene_tree();

        _M_sampler = Package::find_package("Editor", true)->find_object_checked<Sampler>("DefaultSampler");

        ImGuiRenderer::Window::make_current(prev_window);
        return init_world();
    }

    void EditorClient::on_package_select(Package* package)
    {
        if (_M_content_browser)
        {
            _M_content_browser->package = package;
        }
    }

    void EditorClient::on_object_select(Object* object)
    {
        if (_M_properties)
        {
            _M_properties->object = object;
        }
    }

    size_t count = 0;

    ViewportClient& EditorClient::render(class RenderViewport* viewport)
    {
        // Render base frame
        GBuffer::instance()->rhi_bind();
        SceneColorOutput::instance()->rhi_bind();
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    ViewportClient& EditorClient::destroy_script_object(ScriptObject* object)
    {
        if (*object == _M_script_object)
        {
            _M_script_object.remove_reference();
        }
        return *this;
    }


    void EditorClient::render_dock_window(void* userdata)
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

                if (ImGui::MenuItem("editor/Open Package Tree"_localized, nullptr, false, _M_package_tree == nullptr))
                {
                    create_package_tree();
                }

                if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false, _M_content_browser == nullptr))
                {
                    create_content_browser();
                }

                if (ImGui::MenuItem("editor/Open Properties Window"_localized, nullptr, false, _M_properties == nullptr))
                {
                    create_properties_window();
                }

                if (ImGui::MenuItem("editor/Open Scene Tree"_localized, nullptr, false, _M_scene_tree == nullptr))
                {
                    create_scene_tree();
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
                        if (ImGui::MenuItem(localized))
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

            ImGui::DockBuilderDockWindow(ImGuiPackageTree::name(), dock_id_ld);
            ImGui::DockBuilderDockWindow(ImGuiSceneTree::name(), dock_id_left);
            ImGui::DockBuilderDockWindow(ImGuiContentBrowser::name(), dock_id_down);
            ImGui::DockBuilderDockWindow(ImGuiObjectProperties::name(), dock_id_right);
            ImGui::DockBuilderDockWindow(Object::localize("editor/Viewport Title").c_str(), dock_id);

            ImGui::DockBuilderFinish(dock_id);
        }
    }

    ViewportClient& EditorClient::update(class RenderViewport* viewport, float dt)
    {
        ImGuiRenderer::Window* window = viewport->window()->imgui_window();
        window->new_frame();
        make_dock_window("EditorDock", ImGuiWindowFlags_MenuBar, &EditorClient::render_dock_window, this, viewport);

        create_log_window(dt);
        create_viewport_window(dt);

        _M_script_object.update(dt);
        window->end_frame();

        ++_M_frame;

        return *this;
    }

    EditorClient& EditorClient::init_world()
    {
        _M_renderer.scene(World::global()->scene());
        return *this;
    }

    EditorClient& EditorClient::create_log_window(float dt)
    {
        //        if (!ImGui::Begin("Logs"))
        //        {
        //            ImGui::End();
        //            return *this;
        //        }

        //        ImGui::End();
        return *this;
    }

    EditorClient& EditorClient::create_viewport_window(float dt)
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
