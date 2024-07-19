#include <Clients/material_editor_client.hpp>
#include <Clients/open_client.hpp>
#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/group.hpp>
#include <Core/localization.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/properties_window.hpp>
#include <Window/window.hpp>
#include <blueprints.hpp>
#include <editor_config.hpp>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <imgui_stacklayout.h>
#include <shader_compiler.hpp>
#include <theme.hpp>

#define MATERIAL_EDITOR_DEBUG 1

namespace Engine
{
    implement_engine_class_default_init(MaterialEditorClient, 0);


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


    class ImGuiMaterialCode : public ImGuiRenderer::ImGuiAdditionalWindow
    {
    public:
        String code;

        void init(RenderViewport* viewport)
        {}

        bool render(RenderViewport* viewport)
        {
            bool is_open = true;
            ImGui::SetNextWindowSize({300, 400}, ImGuiCond_Appearing);
            ImGui::Begin(name(), &is_open);
            ImGui::InputTextMultiline("##Code", code.data(), code.size(), ImGui::GetContentRegionAvail(),
                                      ImGuiInputTextFlags_ReadOnly);
            ImGui::End();
            return is_open;
        }

        static const char* name()
        {
            return "editor/Material Code"_localized;
        }
    };

    MaterialEditorClient::MaterialEditorClient()
    {
        m_graph_editor_context = ax::NodeEditor::CreateEditor();
    }

    MaterialEditorClient::~MaterialEditorClient()
    {
        ax::NodeEditor::DestroyEditor(m_graph_editor_context);
    }

    MaterialEditorClient& MaterialEditorClient::create_content_browser()
    {
        m_content_browser = ImGuiRenderer::Window::current()->window_list.create<ContentBrowser>();
        m_content_browser->on_close.push([this]() { m_content_browser = nullptr; });
        m_content_browser->on_object_double_click.push(
                std::bind(&MaterialEditorClient::on_object_select, this, std::placeholders::_1));
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::create_preview_window()
    {
        m_preview_window = ImGuiRenderer::Window::current()->window_list.create<ImGuiMaterialPreview>();
        m_preview_window->on_close.push([this]() { m_preview_window = nullptr; });
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::create_material_code_window()
    {
        m_material_code = ImGuiRenderer::Window::current()->window_list.create<ImGuiMaterialCode>();
        m_material_code->on_close.push([this]() { m_material_code = nullptr; });
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::create_properties_window()
    {
        m_properties_window = ImGuiRenderer::Window::current()->window_list.create<ImGuiObjectProperties>();
        m_properties_window->on_close.push([this]() { m_preview_window = nullptr; });
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
        String new_title = Strings::format("Trinex Material Editor [{} RHI]", rhi->info.name.c_str());
        window->title(new_title);

        render_thread()->wait_all();
        m_viewport = viewport;


        ImGuiRenderer::Window* imgui_window = window->imgui_window();
        ImGuiRenderer::Window* prev_window  = ImGuiRenderer::Window::current();
        ImGuiRenderer::Window::make_current(imgui_window);

        create_content_browser().create_preview_window().create_properties_window();

        ImGuiRenderer::Window::make_current(prev_window);
        m_compiler = ShaderCompiler::Compiler::static_create_compiler();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::on_unbind_viewport(class RenderViewport* viewport)
    {
        viewport->window()->imgui_window()->window_list.close_all_windows();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render(class RenderViewport* viewport)
    {
        viewport->rhi_bind();
        viewport->window()->imgui_window()->rhi_render();
        return *this;
    }


    void MaterialEditorClient::on_object_select(Object* object)
    {
        if (Material* material = object->instance_cast<Material>())
        {
            m_material = material;

            if (m_properties_window)
            {
                m_properties_window->update(material);
            }
        }

        if (m_preview_window)
        {
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

                if (ImGui::MenuItem("editor/Open Material Code"_localized, nullptr, nullptr, m_material_code == nullptr))
                {
                    create_material_code_window();
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
                    for (const String& lang : Settings::e_languages)
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
                if (ImGui::MenuItem("Compile source", nullptr, false, m_material != nullptr && m_compiler != 0))
                {
                    m_material->compile(m_compiler, &m_shader_compile_error_list);
                }

                if (ImGui::MenuItem("Just apply", nullptr, false, m_material != nullptr))
                {
                    m_material->apply_changes();
                }

                if (ImGui::MenuItem("Update Source", nullptr, false, m_material != nullptr && m_material_code != nullptr))
                {
                    m_material->shader_source(m_material_code->code);
                }
                ImGui::EndMenu();
            }


            ImGui::EndMenuBar();
        }

        if (ImGui::IsWindowAppearing())
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

            ImGui::DockBuilderDockWindow("###Material Source", dock_id);

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

        ImGui::End();
        viewport->window()->imgui_window()->end_frame();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render_viewport(float dt)
    {
        ImGui::Begin("editor/Material Source###Material Source"_localized);
        render_visual_material_graph(Object::instance_cast<VisualMaterial>(m_material));
        ImGui::End();

        //ImGui::SetNextWindowSize({500, 200}, ImGuiCond_Appearing);
        // ImGui::Begin("Graph Source Code");
        // ImGui::InputTextMultiline("##Text", m_material_source.data(), m_material_source.size(), ImGui::GetContentRegionAvail(),
        //                           ImGuiInputTextFlags_ReadOnly);
        // ImGui::End();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::on_object_dropped(Object* object)
    {
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::update_drag_and_drop()
    {
        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
            if (payload)
            {
                IM_ASSERT(payload->DataSize == sizeof(Object*));
                on_object_dropped(*reinterpret_cast<Object**>(payload->Data));
            }
            ImGui::EndDragDropTarget();
        }
        return *this;
    }
    static InitializeController controller([]() { Object::load_object("Example::TestMaterial"); });
}// namespace Engine
