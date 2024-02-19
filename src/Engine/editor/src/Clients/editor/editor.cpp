#include <Clients/editor_client.hpp>
#include <Clients/open_client.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/localization.hpp>
#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader_parameters.hpp>
#include <ScriptEngine/script_module.hpp>
#include <Systems/event_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Window/window.hpp>
#include <icons.hpp>
#include <imgui_internal.h>
#include <theme.hpp>

namespace Engine
{
    implement_engine_class_default_init(EditorClient);

    EditorClient::EditorClient()
    {
        _M_global_shader_params    = new GlobalShaderParameters();
        _M_global_shader_params_rt = new GlobalShaderParameters();
    }

    EditorClient::~EditorClient()
    {
        EventSystem* event_system = EventSystem::instance();

        if (event_system)
        {
            for (auto listener : _M_event_system_listeners)
            {
                event_system->remove_listener(listener);
            }
        }

        delete _M_global_shader_params;
        delete _M_global_shader_params_rt;
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

    EditorClient& EditorClient::create_content_browser()
    {
        _M_content_browser = ImGuiRenderer::Window::current()->window_list.create<ContentBrowser>();
        _M_content_browser->on_close.push(std::bind(&EditorClient::on_content_browser_close, this));
        _M_content_browser->on_object_select.push(std::bind(&EditorClient::on_object_select, this, std::placeholders::_1));
        return *this;
    }

    EditorClient& EditorClient::create_properties_window()
    {
        _M_properties = ImGuiRenderer::Window::current()->window_list.create<ImGuiObjectProperties>();
        _M_properties->on_close.push(std::bind(&EditorClient::on_properties_window_close, this));

        if (_M_content_browser)
        {
            on_object_select(_M_content_browser->selected_object);
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
        _M_window          = window;
        _M_render_viewport = viewport;

        window->imgui_initialize(initialize_theme);

        String new_title = Strings::format("Trinex Editor [{} RHI]", engine_instance->rhi()->name().c_str());
        window->title(new_title);
        engine_instance->thread(ThreadType::RenderThread)->wait_all();
        EventSystem::new_system<EventSystem>()->process_event_method(EventSystem::PoolEvents);


        ImGuiRenderer::Window* imgui_window = window->imgui_window();
        ImGuiRenderer::Window* prev_window  = ImGuiRenderer::Window::current();
        ImGuiRenderer::Window::make_current(imgui_window);

        create_content_browser();
        create_properties_window();
        create_scene_tree();


        ImGuiRenderer::Window::make_current(prev_window);

        camera                            = Object::new_instance<CameraComponent>();
        camera->transform.rotation_method = Transform::RotationMethod::YXZ;
        camera->transform.location        = {0, 10, 10};
        camera->near_clip_plane           = 0.001;

        EventSystem* event_system = EventSystem::new_system<EventSystem>();
        _M_event_system_listeners.push_back(event_system->add_listener(
                EventType::MouseMotion, std::bind(&EditorClient::on_mouse_move, this, std::placeholders::_1)));

        _M_event_system_listeners.push_back(event_system->add_listener(
                EventType::MouseButtonDown, std::bind(&EditorClient::on_mouse_press, this, std::placeholders::_1)));
        _M_event_system_listeners.push_back(event_system->add_listener(
                EventType::MouseButtonUp, std::bind(&EditorClient::on_mouse_release, this, std::placeholders::_1)));

        _M_event_system_listeners.push_back(event_system->add_listener(
                EventType::KeyDown, std::bind(&EditorClient::on_key_press, this, std::placeholders::_1)));
        _M_event_system_listeners.push_back(event_system->add_listener(
                EventType::KeyUp, std::bind(&EditorClient::on_key_release, this, std::placeholders::_1)));


        extern void render_editor_grid(SceneRenderer * renderer, RenderViewport * viewport, SceneLayer * layer, const CameraView&);
        auto layer = _M_renderer.root_layer()->find(SceneLayer::name_post_process);
        layer->function_callbacks.push_back(render_editor_grid);

        return init_world();
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
        engine_instance->rhi()->push_global_params(*_M_global_shader_params_rt);
        _M_renderer.render(_M_view, _M_render_viewport);

        engine_instance->rhi()->pop_global_params();
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }


    void EditorClient::render_dock_window()
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

            auto dock_id_down  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.25f, nullptr, &dock_id);
            auto dock_id_left  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id);
            auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);

            ImGui::DockBuilderDockWindow(ImGuiSceneTree::name(), dock_id_left);
            ImGui::DockBuilderDockWindow(ContentBrowser::name(), dock_id_down);
            ImGui::DockBuilderDockWindow(ImGuiObjectProperties::name(), dock_id_right);
            ImGui::DockBuilderDockWindow(Object::localize("editor/Viewport Title").c_str(), dock_id);

            ImGui::DockBuilderFinish(dock_id);
        }
    }


    ViewportClient& EditorClient::update(class RenderViewport* viewport, float dt)
    {
        ImGuiRenderer::Window* window = viewport->window()->imgui_window();
        window->new_frame();

        ImGuiViewport* imgui_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(imgui_viewport->WorkPos);
        ImGui::SetNextWindowSize(imgui_viewport->WorkSize);
        ImGui::Begin("EditorDock", nullptr,
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_MenuBar);
        render_dock_window();

        create_log_window(dt);
        create_viewport_window(dt);

        ImGui::End();
        window->end_frame();

        camera->transform.location +=
                Vector3D((camera->transform.rotation_matrix() * Vector4D(_M_camera_move, 1.0))) * dt * _M_camera_speed;
        camera->transform.update();
        _M_global_shader_params->update(SceneColorOutput::instance(), camera);

        struct UpdateParams : ExecutableObject {
            GlobalShaderParameters params;
            GlobalShaderParameters* out = nullptr;

            UpdateParams(GlobalShaderParameters* in, GlobalShaderParameters* out) : params(*in), out(out)
            {}

            int_t execute() override
            {
                *out = params;
                return sizeof(UpdateParams);
            }
        };

        struct UpdateView : ExecutableObject {
            CameraView view;
            CameraView& out;

            UpdateView(const CameraView& in, CameraView& out) : view(in), out(out)
            {}

            int_t execute() override
            {
                out = view;
                return sizeof(UpdateView);
            }
        };

        render_thread()->insert_new_task<UpdateParams>(_M_global_shader_params, _M_global_shader_params_rt);
        render_thread()->insert_new_task<UpdateView>(camera->camera_view(), _M_view);

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
                void* output     = ImGuiRenderer::Window::current()->create_texture(texture, Icons::default_sampler())->handle();
                auto size        = ImGui::GetContentRegionAvail();
                _M_viewport_size = ImGuiHelpers::construct_vec2<Vector2D>(size);
                ImGui::Image(output, size, ImVec2(0, 1), ImVec2(1, 0));
                _M_viewport_is_hovered = ImGui::IsItemHovered();
            }
        }

        ImGui::End();
        return *this;
    }

    // Inputs
    static FORCE_INLINE float calculate_y_rotatation(float offset, float size, float fov)
    {
        return -offset * fov / size;
    }

    void EditorClient::on_mouse_press(const Event& event)
    {
        const MouseButtonEvent& button = event.get<const MouseButtonEvent&>();

        if (_M_viewport_is_hovered && button.button == Mouse::Button::Right)
        {
            MouseSystem::instance()->relative_mode(true, _M_window);
        }
    }

    void EditorClient::on_mouse_release(const Event& event)
    {
        const MouseButtonEvent& button = event.get<const MouseButtonEvent&>();

        if (button.button == Mouse::Button::Right)
        {
            MouseSystem::instance()->relative_mode(false, _M_window);
        }
    }

    void EditorClient::on_mouse_move(const Event& event)
    {
        const MouseMotionEvent& motion = event.get<const MouseMotionEvent&>();

        if (MouseSystem::instance()->is_relative_mode(_M_window))
        {
            camera->transform.rotation.y +=
                    calculate_y_rotatation(static_cast<float>(motion.xrel), _M_viewport_size.x, camera->fov);
            camera->transform.rotation.x +=
                    calculate_y_rotatation(static_cast<float>(motion.yrel), _M_viewport_size.y, camera->fov);
        }
    }

    static FORCE_INLINE void move_camera(Vector3D& move, const KeyEvent& key_event, float factor)
    {
        if (key_event.repeat)
            return;

        float value = 1.f * factor;
        if (key_event.key == Keyboard::W)
        {
            move.z -= value;
        }

        if (key_event.key == Keyboard::A)
        {
            move.x -= value;
        }

        if (key_event.key == Keyboard::S)
        {
            move.z += value;
        }

        if (key_event.key == Keyboard::D)
        {
            move.x += value;
        }
    }

    void EditorClient::on_key_press(const Event& event)
    {
        const KeyEvent& key_event = event.get<const KeyEvent&>();
        move_camera(_M_camera_move, key_event, 1.f);
    }


    void EditorClient::on_key_release(const Event& event)
    {
        const KeyEvent& key_event = event.get<const KeyEvent&>();
        move_camera(_M_camera_move, key_event, -1.f);
    }
}// namespace Engine
