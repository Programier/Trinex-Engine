#include <Clients/material_editor_client.hpp>
#include <Clients/open_client.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/group.hpp>
#include <Core/localization.hpp>
#include <Core/render_thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Window/window.hpp>
#include <editor_config.hpp>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <theme.hpp>

#define MATERIAL_EDITOR_DEBUG 1

namespace Engine
{
    implement_engine_class_default_init(MaterialEditorClient);


    class ImGuiMaterialPreview : public ImGuiRenderer::ImGuiAdditionalWindow
    {
    public:
        void init(RenderViewport* viewport)
        {}

        bool render(RenderViewport* viewport)
        {
            bool is_open = true;
            ImGui::Begin(name(), &is_open);
            ImGui::End();
            return is_open;
        }

        static const char* name()
        {
            return "editor/MaterialPreview"_localized;
        }
    };

    void MaterialEditorClient::on_content_browser_close()
    {
        m_content_browser = nullptr;
    }

    void MaterialEditorClient::on_properties_window_close()
    {
        m_properties = nullptr;
    }

    void MaterialEditorClient::on_preview_close()
    {
        m_preview_window = nullptr;
    }


    MaterialEditorClient& MaterialEditorClient::create_content_browser()
    {
        m_content_browser = ImGuiRenderer::Window::current()->window_list.create<ContentBrowser>();
        m_content_browser->on_close.push(std::bind(&MaterialEditorClient::on_content_browser_close, this));
        m_content_browser->on_object_double_click.push(
                std::bind(&MaterialEditorClient::on_object_select, this, std::placeholders::_1));
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::create_properties_window()
    {
        m_properties = ImGuiRenderer::Window::current()->window_list.create<ImGuiObjectProperties>();
        m_properties->on_close.push(std::bind(&MaterialEditorClient::on_properties_window_close, this));

        if (m_content_browser)
        {
            on_object_select(m_content_browser->selected_object);
        }
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::create_preview_window()
    {
        m_preview_window = ImGuiRenderer::Window::current()->window_list.create<ImGuiMaterialPreview>();
        m_preview_window->on_close.push(std::bind(&MaterialEditorClient::on_preview_close, this));
        return *this;
    }


    MaterialEditorClient& MaterialEditorClient::on_bind_viewport(class RenderViewport* viewport)
    {
        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot bind client to non-window viewport!");
        }

        window->imgui_initialize(initialize_theme);
        String new_title = Strings::format("Trinex Material Editor [{} RHI]", engine_instance->rhi()->name().c_str());
        window->title(new_title);

        engine_instance->thread(ThreadType::RenderThread)->wait_all();
        m_viewport = viewport;


        ImGuiRenderer::Window* imgui_window = window->imgui_window();
        ImGuiRenderer::Window* prev_window  = ImGuiRenderer::Window::current();
        ImGuiRenderer::Window::make_current(imgui_window);

        create_properties_window().create_content_browser().create_preview_window();

        // Create imgui node editor context
        ax::NodeEditor::Config config;
        config.NavigateButtonIndex = ImGuiMouseButton_Middle;
        config.SettingsFile        = nullptr;

        auto context = ax::NodeEditor::CreateEditor(&config);
        imgui_window->on_destroy.push([context]() { ax::NodeEditor::DestroyEditor(context); });


        m_editor_context = context;

        ImGuiRenderer::Window::make_current(prev_window);

        Class* instance = Class::static_find(editor_config.material_compiler);
        if (instance)
        {
        }

        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    ImGuiObjectProperties* MaterialEditorClient::properties_window() const
    {
        return m_properties;
    }

    void MaterialEditorClient::on_object_select(Object* object)
    {
        if (m_preview_window)
        {
        }

        if (m_properties)
        {
            m_properties->update(object);
        }
    }

    void MaterialEditorClient::render_dock_window()
    {
        auto dock_id                       = ImGui::GetID("MaterialEditorDock##Dock");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("editor/View"_localized))
            {
                if (ImGui::MenuItem("editor/Open Editor"_localized))
                {
                    open_editor();
                }

                if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false, m_content_browser == nullptr))
                {
                    create_content_browser();
                }

                if (ImGui::MenuItem("editor/Open Properties Window"_localized, nullptr, false, m_properties == nullptr))
                {
                    create_properties_window();
                }

                ImGui::Checkbox("editor/Open Material Code"_localized, &m_open_material_code_window);
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

            if (ImGui::BeginMenu("editor/Material"_localized))
            {
                ImGui::EndMenu();
            }


            ImGui::EndMenuBar();
        }

        if (m_frame == 0)
        {
            ImGui::DockBuilderRemoveNode(dock_id);
            ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);


            auto dock_id_down  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.3f, nullptr, &dock_id);
            auto dock_id_left  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id);
            auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.2f, nullptr, &dock_id);

            ImGui::DockBuilderDockWindow(ContentBrowser::name(), dock_id_down);
            ImGui::DockBuilderDockWindow(ImGuiMaterialPreview::name(), dock_id_left);
            ImGui::DockBuilderDockWindow(ImGuiObjectProperties::name(), dock_id_right);

            ImGui::DockBuilderDockWindow("editor/Material Graph###Material Graph"_localized, dock_id);

            ImGui::DockBuilderFinish(dock_id);
        }
    }

    MaterialEditorClient& MaterialEditorClient::update(class RenderViewport* viewport, float dt)
    {
        viewport->window()->imgui_window()->new_frame();
        ImGuiViewport* imgui_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(imgui_viewport->WorkPos);
        ImGui::SetNextWindowSize(imgui_viewport->WorkSize);

        ImGui::Begin("MaterialEditorDock", nullptr,
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_MenuBar);
        render_dock_window();

        render_viewport(dt);
        render_material_code();

        ImGui::End();
        viewport->window()->imgui_window()->end_frame();
        ++m_frame;
        return *this;
    }

    static Struct* render_node_types(Group* group)
    {
        if (!group)
            return nullptr;


        Struct* current = nullptr;

        for (Group* child : group->childs())
        {
            if (ImGui::CollapsingHeader(child->name().c_str()))
            {
                ImGui::Indent(10.f);
                Struct* new_struct = render_node_types(child);
                if (!current && new_struct)
                    current = new_struct;

                ImGui::Unindent(10.f);
            }
        }


        for (Struct* instance : group->structs())
        {
            if (ImGui::Selectable(instance->base_name().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (!current && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    current = instance;
                }
            }
        }

        return current;
    }

    static Struct* render_nodes_window()
    {
        static Group* root_group = Group::find("Engine::VisualMaterialNodes");
        return render_node_types(root_group);
    }

    MaterialEditorClient& MaterialEditorClient::render_viewport(float dt)
    {
        ImGui::Begin("editor/Material Graph###Material Graph"_localized);

        if (!ImGui::IsWindowAppearing())
        {
            ax::NodeEditor::SetCurrentEditor(reinterpret_cast<ax::NodeEditor::EditorContext*>(m_editor_context));

            ax::NodeEditor::Begin("###Viewport");
            ax::NodeEditor::End();

            ImGui::End();
            return *this;


            update_drag_and_drop();

            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                m_open_select_node_window = true;
            }


            if (m_open_select_node_window)
            {
                const char* name = "editor/Menu"_localized;

                ImGui::SetNextWindowSize({300, 350}, ImGuiCond_Appearing);
                ImGui::SetNextWindowSizeConstraints({300, 350}, {FLT_MAX, FLT_MAX});
                auto pos = ImGui::GetMousePos();
                ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);

                ImGui::Begin(name, &m_open_select_node_window, ImGuiWindowFlags_NoSavedSettings);

                if (ImGui::IsWindowAppearing())
                {
                    m_next_node_pos = ImGuiHelpers::construct_vec2<Vector2D>(ax::NodeEditor::ScreenToCanvas(pos));
                }

                {
                    Struct* selected = render_nodes_window();

                    if (selected)
                    {
                        m_open_select_node_window = false;
                    }
                }

                ImGui::End();
            }


            ax::NodeEditor::SetCurrentEditor(nullptr);
        }
        ImGui::End();
        return *this;
    }

    void MaterialEditorClient::render_material_code()
    {}

    void* MaterialEditorClient::editor_context() const
    {
        return m_editor_context;
    }

    MaterialEditorClient& MaterialEditorClient::on_object_dropped(Object* object)
    {
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::update_drag_and_drop()
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
}// namespace Engine
