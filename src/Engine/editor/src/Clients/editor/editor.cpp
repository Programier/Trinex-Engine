#include <Clients/editor_client.hpp>
#include <Clients/open_client.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/localization.hpp>
#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/scene.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Engine/world.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader_parameters.hpp>
#include <ImGuizmo.h>
#include <Importer/importer.hpp>
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
        m_global_shader_params    = new GlobalShaderParameters();
        m_global_shader_params_rt = new GlobalShaderParameters();
    }

    EditorClient::~EditorClient()
    {
        EventSystem* event_system = EventSystem::instance();

        if (event_system)
        {
            for (auto listener : m_event_system_listeners)
            {
                event_system->remove_listener(listener);
            }
        }

        delete m_global_shader_params;
        delete m_global_shader_params_rt;
    }

    void EditorClient::on_content_browser_close()
    {
        m_content_browser = nullptr;
    }

    void EditorClient::on_properties_window_close()
    {
        m_properties = nullptr;
    }

    void EditorClient::on_scene_tree_close()
    {
        m_scene_tree = nullptr;
    }

    EditorClient& EditorClient::create_content_browser()
    {
        m_content_browser = ImGuiRenderer::Window::current()->window_list.create<ContentBrowser>();
        m_content_browser->on_close.push(std::bind(&EditorClient::on_content_browser_close, this));
        m_content_browser->on_object_select.push(std::bind(&EditorClient::on_object_select, this, std::placeholders::_1));
        return *this;
    }

    EditorClient& EditorClient::create_properties_window()
    {
        m_properties = ImGuiRenderer::Window::current()->window_list.create<ImGuiObjectProperties>();
        m_properties->on_close.push(std::bind(&EditorClient::on_properties_window_close, this));

        if (m_content_browser)
        {
            on_object_select(m_content_browser->selected_object);
        }
        return *this;
    }

    EditorClient& EditorClient::create_scene_tree()
    {
        m_scene_tree = ImGuiRenderer::Window::current()->window_list.create<ImGuiSceneTree>();
        m_scene_tree->on_close.push(std::bind(&EditorClient::on_scene_tree_close, this));
        m_scene_tree->on_node_select.push(
                [this](SceneComponent* component) { on_object_select(static_cast<Object*>(component)); });

        if (m_world)
        {
            m_scene_tree->root_component = m_world->scene()->root_component();
        }
        return *this;
    }

    ViewportClient& EditorClient::on_bind_to_viewport(class RenderViewport* viewport)
    {
        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot bind client to non-window viewport!");
        }
        m_window          = window;
        m_render_viewport = viewport;

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
        camera->near_clip_plane           = 0.1;
        camera->far_clip_plane            = 1000.f;

        EventSystem* event_system = EventSystem::new_system<EventSystem>();
        m_event_system_listeners.push_back(event_system->add_listener(
                EventType::MouseMotion, std::bind(&EditorClient::on_mouse_move, this, std::placeholders::_1)));

        m_event_system_listeners.push_back(event_system->add_listener(
                EventType::MouseButtonDown, std::bind(&EditorClient::on_mouse_press, this, std::placeholders::_1)));
        m_event_system_listeners.push_back(event_system->add_listener(
                EventType::MouseButtonUp, std::bind(&EditorClient::on_mouse_release, this, std::placeholders::_1)));

        m_event_system_listeners.push_back(event_system->add_listener(
                EventType::KeyDown, std::bind(&EditorClient::on_key_press, this, std::placeholders::_1)));
        m_event_system_listeners.push_back(event_system->add_listener(
                EventType::KeyUp, std::bind(&EditorClient::on_key_release, this, std::placeholders::_1)));

        return init_world();
    }

    void EditorClient::on_object_select(Object* object)
    {
        if (m_properties)
        {
            m_properties->object = object;
        }
    }

    EditorClient& EditorClient::update_drag_and_drop()
    {
        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContendBrowser->Object");
            if (payload)
            {
                IM_ASSERT(payload->DataSize == sizeof(Object*));
                on_object_dropped(*reinterpret_cast<Object**>(payload->Data));
            }
            ImGui::EndDragDropTarget();
        }
        return *this;
    }

    ViewportClient& EditorClient::render(class RenderViewport* viewport)
    {
        engine_instance->rhi()->push_global_params(*m_global_shader_params_rt);
        m_renderer.render(m_view, m_render_viewport, m_viewport_size);

        engine_instance->rhi()->pop_global_params();
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }


    void EditorClient::render_dock_window(float dt)
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

                if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false, m_content_browser == nullptr))
                {
                    create_content_browser();
                }

                if (ImGui::MenuItem("editor/Open Properties Window"_localized, nullptr, false, m_properties == nullptr))
                {
                    create_properties_window();
                }

                if (ImGui::MenuItem("editor/Open Scene Tree"_localized, nullptr, false, m_scene_tree == nullptr))
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

            if (ImGui::BeginMenu("editor/Import"_localized))
            {
                bool enable_import = m_content_browser && m_content_browser->selected_package() != nullptr;
                if (ImGui::MenuItem("editor/Import resource"_localized,
                                    "editor/Import resource from file to selected package"_localized, false, enable_import))
                {
                    ImGuiRenderer::Window::current()->window_list.create<ImGuiOpenFile>(m_content_browser->selected_package(),
                                                                                        Importer::import_resource);
                }

                ImGui::EndMenu();
            }

            ImGui::Text("FPS: %f\n", 1.f / dt);
            ImGui::EndMenuBar();
        }

        if (m_frame == 0)
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
        render_dock_window(dt);

        create_log_window(dt);
        create_viewport_window(dt);

        ImGui::End();
        window->end_frame();

        camera->transform.location +=
                Vector3D((camera->transform.rotation_matrix() * Vector4D(m_camera_move, 1.0))) * dt * m_camera_speed;
        camera->transform.update();
        m_global_shader_params->update(SceneColorOutput::instance(), camera);

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

        render_thread()->insert_new_task<UpdateParams>(m_global_shader_params, m_global_shader_params_rt);
        render_thread()->insert_new_task<UpdateView>(camera->camera_view(), m_view);

        ++m_frame;
        return *this;
    }

    EditorClient& EditorClient::init_world()
    {
        m_world      = World::new_system<World>();
        Scene* scene = m_world->scene();
        m_renderer.scene(scene);

        extern void render_editor_grid(SceneRenderer * renderer, RenderViewport * viewport, SceneLayer * layer);
        auto layer = scene->clear_layer()->create_next("Grid Rendering");
        layer->function_callbacks.push_back(render_editor_grid);
        m_scene_tree->root_component = m_world->scene()->root_component();
        m_world->start_play();
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

        Texture* texture = nullptr;
        if (m_target_view_index == 0)
        {
            auto* frame = SceneColorOutput::instance()->current_frame();
            if (frame)
            {
                texture = frame->texture();
            }
        }
        else
        {
            auto* frame = GBuffer::instance()->current_frame();

            if (frame)
            {
                switch (m_target_view_index)
                {
                    case 1:
                        texture = frame->base_color();
                        break;
                    case 2:
                        texture = frame->position();
                        break;
                    case 3:
                        texture = frame->normal();
                        break;
                    case 4:
                        texture = frame->emissive();
                        break;
                    case 5:
                        texture = frame->data_buffer();
                        break;
                    case 6:
                        texture = frame->depth();
                        break;
                    default:
                        break;
                }
            }
        }

        if (texture && texture->has_object())
        {
            {
                void* output    = ImGuiRenderer::Window::current()->create_texture(texture, Icons::default_sampler())->handle();
                auto size       = ImGui::GetContentRegionAvail();
                m_viewport_size = ImGuiHelpers::construct_vec2<Vector2D>(size);
                camera->aspect_ratio = m_viewport_size.x / m_viewport_size.y;
                ImGui::Image(output, size, ImVec2(0, 1), ImVec2(1, 0));
                m_viewport_is_hovered = ImGui::IsItemHovered();
            }

            update_drag_and_drop();
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

        if (m_viewport_is_hovered && button.button == Mouse::Button::Right)
        {
            MouseSystem::instance()->relative_mode(true, m_window);
        }
    }

    void EditorClient::on_mouse_release(const Event& event)
    {
        const MouseButtonEvent& button = event.get<const MouseButtonEvent&>();

        if (button.button == Mouse::Button::Right)
        {
            MouseSystem::instance()->relative_mode(false, m_window);
        }
    }

    void EditorClient::on_mouse_move(const Event& event)
    {
        const MouseMotionEvent& motion = event.get<const MouseMotionEvent&>();

        if (MouseSystem::instance()->is_relative_mode(m_window))
        {
            camera->transform.rotation.y +=
                    calculate_y_rotatation(static_cast<float>(motion.xrel), m_viewport_size.x, camera->fov);
            camera->transform.rotation.x +=
                    calculate_y_rotatation(static_cast<float>(motion.yrel), m_viewport_size.y, camera->fov);
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
        if (event.window_id() != m_window->window_id())
            return;

        const KeyEvent& key_event = event.get<const KeyEvent&>();
        move_camera(m_camera_move, key_event, 1.f);
    }


    void EditorClient::on_key_release(const Event& event)
    {
        if (event.window_id() != m_window->window_id())
            return;

        const KeyEvent& key_event = event.get<const KeyEvent&>();
        move_camera(m_camera_move, key_event, -1.f);

        if (static_cast<Identifier>(key_event.key) >= static_cast<Identifier>(Keyboard::Key::Num0) &&
            static_cast<Identifier>(key_event.key) <= static_cast<Identifier>(Keyboard::Key::Num9))
        {

            Index new_index = static_cast<Identifier>(key_event.key) - static_cast<Identifier>(Keyboard::Key::Num0);
            if (new_index <= 6)
            {
                m_target_view_index = new_index;
            }
        }
    }
}// namespace Engine
