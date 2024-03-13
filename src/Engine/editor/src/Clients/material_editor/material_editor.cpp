#include <ShaderCompiler/compiler.hpp>

#include <Clients/material_editor_client.hpp>
#include <Clients/open_client.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/group.hpp>
#include <Core/localization.hpp>
#include <Core/render_thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <PropertyRenderers/imgui_class_property.hpp>

#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Window/window.hpp>
#include <editor_config.hpp>
#include <imgui_internal.h>
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

        create_content_browser().create_preview_window();

        ImGuiRenderer::Window::make_current(prev_window);
        Class* instance = Class::static_find(Strings::format("Engine::ShaderCompiler::{}_ShaderCompiler", engine_config.api));

        if (instance)
        {
            m_compiler = instance->create_object()->instance_cast<ShaderCompiler::ShaderCompiler>();
        }

        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render_properties()
    {
        ImGui::Begin("editor/Properties Title"_localized);

        if (m_material == nullptr)
        {
            ImGui::End();
            return *this;
        }

        ImGui::SeparatorText("Pipeline");
        render_object_properties(m_material->pipeline, true);

        ImGui::End();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::submit_compiled_source(const ShaderCompiler::ShaderSource& source)
    {
        if (!m_material)
            return *this;

        auto vertex_shader   = m_material->pipeline->vertex_shader;
        auto fragment_shader = m_material->pipeline->fragment_shader;
        vertex_shader->attributes.clear();
        vertex_shader->attributes.reserve(source.reflection.attributes.size());


        for (auto& attribute : source.reflection.attributes)
        {
            VertexShader::Attribute out_attribute;
            out_attribute.name           = attribute.name;
            out_attribute.format         = attribute.format;
            out_attribute.rate           = attribute.rate;
            out_attribute.semantic       = attribute.semantic;
            out_attribute.semantic_index = attribute.semantic_index;
            out_attribute.count          = attribute.count;

            vertex_shader->attributes.push_back(out_attribute);
        }

        vertex_shader->source_code   = source.vertex_code;
        fragment_shader->source_code = source.fragment_code;


        m_material->apply_changes();
        return *this;
    }

    void MaterialEditorClient::on_object_select(Object* object)
    {
        if (Material* material = object->instance_cast<Material>())
        {
            m_material = material;
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
                if (ImGui::MenuItem("Compile source", nullptr, false, m_material != nullptr && m_compiler != 0))
                {
                    ShaderCompiler::ShaderSource source;

                    if (m_compiler->compile(m_material, source, m_shader_compile_error_list))
                    {
                        submit_compiled_source(source);
                    }
                }

                if (ImGui::MenuItem("Just apply", nullptr, false, m_material != nullptr))
                {
                    m_material->apply_changes();
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
            ImGui::DockBuilderDockWindow("editor/Properties Title"_localized, dock_id_right);

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
        render_properties();

        ImGui::End();
        viewport->window()->imgui_window()->end_frame();
        return *this;
    }

    static int input_text_callback(ImGuiInputTextCallbackData* data)
    {

        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            Buffer* buffer     = reinterpret_cast<Buffer*>(data->UserData);
            const int new_size = data->BufTextLen;
            buffer->resize(new_size+3);
            data->Buf = (char*) buffer->data();
        }

        return 0;
    }

    MaterialEditorClient& MaterialEditorClient::render_viewport(float dt)
    {
        ImGui::Begin("editor/Material Source###Material Source"_localized);

        if (ImGui::BeginTabBar("Source"))
        {
            if (ImGui::BeginTabItem("Vertex"))
            {
                if (m_material && !m_material->pipeline->vertex_shader->source_code.empty())
                {
                    ImGui::BeginChild(ImGui::GetID(m_material->pipeline->vertex_shader), ImGui::GetContentRegionAvail());
                    ImGui::InputTextMultiline("##source", (char*) m_material->pipeline->vertex_shader->source_code.data(),
                                              m_material->pipeline->vertex_shader->source_code.size(),
                                              ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_CallbackResize, input_text_callback,
                                              &m_material->pipeline->vertex_shader->source_code);
                    ImGui::EndChild();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Fragment"))
            {
                if (m_material && !m_material->pipeline->fragment_shader->source_code.empty())
                {
                    ImGui::BeginChild(ImGui::GetID(m_material->pipeline->fragment_shader), ImGui::GetContentRegionAvail());
                    ImGui::InputTextMultiline("##source", (char*) m_material->pipeline->fragment_shader->source_code.data(),
                                              m_material->pipeline->fragment_shader->source_code.size(),
                                              ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_CallbackResize, input_text_callback,
                                              &m_material->pipeline->fragment_shader->source_code);
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Errors"))
            {
                ImGui::BeginChild(ImGui::GetID(&m_shader_compile_error_list), ImGui::GetContentRegionAvail());
                for (auto& entry : m_shader_compile_error_list)
                {
                    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "%s", entry.c_str());
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
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
