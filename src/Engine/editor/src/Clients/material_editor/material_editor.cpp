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
#include <Graphics/visual_material.hpp>
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

    MaterialEditorClient::MaterialEditorClient()
    {
        m_graph_editor_context = ax::NodeEditor::CreateEditor();
    }

    MaterialEditorClient::~MaterialEditorClient()
    {
        ax::NodeEditor::DestroyEditor(m_graph_editor_context);
    }

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
        m_preview_window->on_close.push([this]() { m_preview_window = nullptr; });
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::create_properties_window()
    {
        m_properties_window = ImGuiRenderer::Window::current()->window_list.create<ImGuiObjectProperties>();
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

        create_content_browser().create_preview_window().create_properties_window();

        ImGuiRenderer::Window::make_current(prev_window);
        Class* instance = Class::static_find(Strings::format("Engine::ShaderCompiler::{}_Compiler", engine_config.api));

        if (instance)
        {
            m_compiler = instance->create_object()->instance_cast<ShaderCompiler::Compiler>();
        }
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
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
                    m_material->compile(m_compiler, &m_shader_compile_error_list);
                }

                if (ImGui::MenuItem("Just apply", nullptr, false, m_material != nullptr))
                {
                    m_material->apply_changes();
                }

                VisualMaterial* material = Object::instance_cast<VisualMaterial>(m_material);
                if (ImGui::MenuItem("Update Source", nullptr, false, material != nullptr))
                {
                    material->shader_source(m_material_source);
//                    auto link = material->nodes()[0]->inputs()[0]->linked_to();
//                    if (link)
//                    {
//                        material->reset_nodes_state();
//                        VisualMaterial::CompilerState state;

//                        String pin_source = link->node()->compile(link, state).code;

//                        m_material_source.clear();

//                        for (auto& local : state.locals)
//                        {
//                            m_material_source += local;
//                            m_material_source.push_back('\n');
//                        }
//                        m_material_source += pin_source;
//                    }
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
        ax::NodeEditor::SetCurrentEditor(m_graph_editor_context);
        ax::NodeEditor::Begin("Editor");

        if (VisualMaterial* visual_material = Object::instance_cast<VisualMaterial>(m_material))
        {
            render_visual_material_graph(visual_material);
        }

        ax::NodeEditor::End();

        ImGui::End();

        ImGui::SetNextWindowSize({500, 200}, ImGuiCond_Appearing);
        ImGui::Begin("Graph Source Code");
        ImGui::InputTextMultiline("##Text", m_material_source.data(), m_material_source.size(), ImGui::GetContentRegionAvail(),
                                  ImGuiInputTextFlags_ReadOnly);
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


    //////////////////////////////////// GRAPH RENDERING ////////////////////////////////////

    static void show_label(const char* label, ImColor color)
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
        auto size = ImGui::CalcTextSize(label);

        auto padding = ImGui::GetStyle().FramePadding;
        auto spacing = ImGui::GetStyle().ItemSpacing;

        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

        auto rectMin = ImGui::GetCursorScreenPos() - padding;
        auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

        auto drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
        ImGui::TextUnformatted(label);
    };

    static const TreeMap<VisualMaterial::PinType, ImVec4> pin_colors = {
            {VisualMaterial::PinType::Undefined, ImVec4(0.0, 0.0, 1.0, 1.0)},
            {VisualMaterial::PinType::Bool, ImVec4(0.0, 0.0, 1.0, 1.0)},
            {VisualMaterial::PinType::Int, ImVec4(1.0, 0.0, 0.0, 1.0)},
            {VisualMaterial::PinType::UInt, ImVec4(1.0, 0.647, 0.0, 1.0)},
            {VisualMaterial::PinType::Float, ImVec4(0.0, 1.0, 0.0, 1.0)},
            {VisualMaterial::PinType::BVec2, ImVec4(1.0, 0.753, 0.796, 1.0)},
            {VisualMaterial::PinType::BVec3, ImVec4(1.0, 0.078, 0.576, 1.0)},
            {VisualMaterial::PinType::BVec4, ImVec4(1.0, 0.412, 0.706, 1.0)},
            {VisualMaterial::PinType::IVec2, ImVec4(1.0, 1.0, 0.0, 1.0)},
            {VisualMaterial::PinType::IVec3, ImVec4(0.855, 0.647, 0.125, 1.0)},
            {VisualMaterial::PinType::IVec4, ImVec4(1.0, 0.843, 0.0, 1.0)},
            {VisualMaterial::PinType::UVec2, ImVec4(0.0, 1.0, 1.0, 1.0)},
            {VisualMaterial::PinType::UVec3, ImVec4(0.255, 0.412, 0.882, 1.0)},
            {VisualMaterial::PinType::UVec4, ImVec4(0.0, 0.749, 1.0, 1.0)},
            {VisualMaterial::PinType::Vec2, ImVec4(1.0, 0.0, 1.0, 1.0)},
            {VisualMaterial::PinType::Vec3, ImVec4(0.502, 0.0, 0.502, 1.0)},
            {VisualMaterial::PinType::Color3, ImVec4(1.0, 0.078, 0.576, 1.0)},
            {VisualMaterial::PinType::Vec4, ImVec4(0.502, 0.0, 0.0, 1.0)},
            {VisualMaterial::PinType::Color4, ImVec4(1.0, 0.412, 0.706, 1.0)},
            {VisualMaterial::PinType::Mat3, ImVec4(1.0, 0.843, 0.0, 1.0)},
            {VisualMaterial::PinType::Mat4, ImVec4(0.855, 0.647, 0.125, 1.0)},
            {VisualMaterial::PinType::Sampler, ImVec4(0.0, 0.502, 0.502, 1.0)},
            {VisualMaterial::PinType::CombinedImageSampler2D, ImVec4(0.0, 1.0, 1.0, 1.0)},
            {VisualMaterial::PinType::Texture2D, ImVec4(0.0, 0.502, 0.0, 1.0)},
    };

    static FORCE_INLINE float input_item_width(int components)
    {
        return 75.f * static_cast<float>(components);
    }

    static float render_default_value(void* data, VisualMaterial::PinType type)
    {
        if (data == nullptr)
            return 0;

        union Storage
        {
            void* ptr;
            bool* bool_ptr;
            float* float_ptr;

            Storage(void* data) : ptr(data)
            {}
        } storage(data);

        ImGui::BeginGroup();

        switch (type)
        {
            case VisualMaterial::PinType::Bool:
                ImGui::Checkbox("##Value", storage.bool_ptr);
                break;

            case VisualMaterial::PinType::Int:
                ImGui::SetNextItemWidth(input_item_width(1));
                ImGui::InputScalar("##Value", ImGuiDataType_S32, storage.ptr);
                break;

            case VisualMaterial::PinType::UInt:
                ImGui::SetNextItemWidth(input_item_width(1));
                ImGui::InputScalar("##Value", ImGuiDataType_U32, storage.ptr);
                break;

            case VisualMaterial::PinType::Float:
                ImGui::SetNextItemWidth(input_item_width(1));
                ImGui::InputScalar("##Value", ImGuiDataType_Float, storage.ptr);
                break;

            case VisualMaterial::PinType::BVec2:
                ImGui::SetNextItemWidth(input_item_width(2));
                ImGui::Checkbox("##Value1", storage.bool_ptr);
                ImGui::Checkbox("##Value2", storage.bool_ptr + 1);
                break;

            case VisualMaterial::PinType::BVec3:
                ImGui::SetNextItemWidth(input_item_width(3));
                ImGui::Checkbox("##Value1", storage.bool_ptr);
                ImGui::Checkbox("##Value2", storage.bool_ptr + 1);
                ImGui::Checkbox("##Value3", storage.bool_ptr + 2);
                break;

            case VisualMaterial::PinType::BVec4:
                ImGui::SetNextItemWidth(input_item_width(4));
                ImGui::Checkbox("##Value1", storage.bool_ptr);
                ImGui::Checkbox("##Value2", storage.bool_ptr + 1);
                ImGui::Checkbox("##Value3", storage.bool_ptr + 2);
                ImGui::Checkbox("##Value4", storage.bool_ptr + 3);
                break;
            case VisualMaterial::PinType::IVec2:
                ImGui::SetNextItemWidth(input_item_width(2));
                ImGui::InputScalarN("##Value", ImGuiDataType_S32, storage.ptr, 2);
                break;
            case VisualMaterial::PinType::IVec3:
                ImGui::SetNextItemWidth(input_item_width(3));
                ImGui::InputScalarN("##Value", ImGuiDataType_S32, storage.ptr, 3);
                break;
            case VisualMaterial::PinType::IVec4:
                ImGui::SetNextItemWidth(input_item_width(4));
                ImGui::InputScalarN("##Value", ImGuiDataType_S32, storage.ptr, 4);
                break;
            case VisualMaterial::PinType::UVec2:
                ImGui::SetNextItemWidth(input_item_width(2));
                ImGui::InputScalarN("##Value", ImGuiDataType_U32, storage.ptr, 2);
                break;
            case VisualMaterial::PinType::UVec3:
                ImGui::SetNextItemWidth(input_item_width(3));
                ImGui::InputScalarN("##Value", ImGuiDataType_U32, storage.ptr, 3);
                break;
            case VisualMaterial::PinType::UVec4:
                ImGui::SetNextItemWidth(input_item_width(4));
                ImGui::InputScalarN("##Value", ImGuiDataType_U32, storage.ptr, 4);
                break;
            case VisualMaterial::PinType::Vec2:
                ImGui::SetNextItemWidth(input_item_width(2));
                ImGui::InputScalarN("##Value", ImGuiDataType_Float, storage.ptr, 2);
                break;
            case VisualMaterial::PinType::Vec3:
                ImGui::SetNextItemWidth(input_item_width(3));
                ImGui::InputScalarN("##Value", ImGuiDataType_Float, storage.ptr, 3);
                break;
            case VisualMaterial::PinType::Color3:
                ImGui::SetNextItemWidth(input_item_width(3));
                ImGui::ColorEdit3("##Value", storage.float_ptr);
                break;
            case VisualMaterial::PinType::Vec4:
                ImGui::SetNextItemWidth(input_item_width(4));
                ImGui::InputScalarN("##Value", ImGuiDataType_Float, storage.ptr, 4);
                break;
            case VisualMaterial::PinType::Color4:
                ImGui::SetNextItemWidth(input_item_width(4));
                ImGui::ColorEdit4("##Value", storage.float_ptr);
                break;
            case VisualMaterial::PinType::Mat3:
                break;
            case VisualMaterial::PinType::Mat4:
                break;
            default:
                break;
        }

        ImGui::EndGroup();
        return ImGui::GetItemRectSize().x;
    }


    static void render_graph(const Vector<VisualMaterial::Node*>& nodes)
    {
        static BlueprintBuilder builder;

        float text_height  = ImGui::GetTextLineHeightWithSpacing();
        float item_spacing = ImGui::GetStyle().ItemSpacing.x;

        for (auto& node : nodes)
        {
            builder.begin(node->id());

            builder.begin_header(ImGuiHelpers::construct_vec2<ImVec4>(node->header_color()));
            ImGui::Spring(1.f);
            ImGui::TextUnformatted(node->name());
            ImGui::Dummy({0, ImGui::GetTextLineHeightWithSpacing()});
            ImGui::Spring(1.f);
            builder.end_header();

            {
                float max_len = 0.f;
                static Vector<float> sizes;
                sizes.clear();

                for (auto& input : node->inputs())
                {
                    float len = ImGui::CalcTextSize(input->name().c_str()).x;
                    max_len   = glm::max(max_len, len);
                    sizes.push_back(len);
                }

                Index index = 0;
                for (auto& input : node->inputs())
                {
                    builder.begin_input(input->id());

                    ImGui::Spring(0.f, 0.f);
                    builder.begin_input_pin(input->id());
                    BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, input->has_links(),
                                           pin_colors.at(input->type()));
                    builder.end_input_pin();

                    ImGui::Spring(0.f, 0.f);
                    ImGui::TextUnformatted(input->name().c_str());

                    ImGui::Spring(0.f, 0.f);
                    ImGui::Dummy({item_spacing + max_len - sizes[index], 0.f});
                    ImGui::Spring(0.f, 0.f);

                    ImGui::SuspendLayout();
                    float width = render_default_value(input->default_value(), input->type());
                    ImGui::ResumeLayout();

                    ImGui::Spring(0.f, 0.f);
                    ImGui::Dummy({width, 0.f});
                    ImGui::Spring(1.f, 0.f);

                    builder.end_input();

                    ++index;
                }
            }
            //            ImGui::EndVertical();

            //            ImGui::BeginVertical("test");
            //            for (int i = 0; i < 10; i++)
            //            {
            //                ImGui::TextUnformatted("Test");
            //            }
            //            ImGui::EndVertical();


            for (auto& output : node->outputs())
            {
                builder.begin_output(output->id());

                ImGui::Spring(0.f, 0.f);
                ImGui::SuspendLayout();
                float width = render_default_value(output->default_value(), output->type());
                ImGui::ResumeLayout();

                ImGui::Spring(0.f, 0.f);
                ImGui::Dummy({width, 0.f});
                ImGui::Spring(1.f, 0.f);

                ImGui::TextUnformatted(output->name().c_str());
                ImGui::Spring(0.f, 0.f);

                builder.begin_output_pin(output->id());
                BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, output->has_links(),
                                       pin_colors.at(output->type()));
                builder.end_output_pin();

                ImGui::Spring(0.f, 0.f);
                builder.end_output();
            }

            builder.end();
        }

        for (VisualMaterial::Node* node : nodes)
        {
            for (auto* input : node->inputs())
            {
                if (auto* output = input->linked_to())
                {
                    ed::Link(input->id() + 1, input->id(), output->id(), ImColor(0, 149, 220), 2.f);
                }
            }
        }
    }


    static void check_creating(void*& out_pin)
    {
        if (ed::BeginCreate(ImColor(0, 169, 233), 2.f))
        {
            ed::PinId from, to;
            if (ed::QueryNewLink(&from, &to) && from && to)
            {
                VisualMaterial::Pin* input_pin  = from.AsPointer<VisualMaterial::Pin>();
                VisualMaterial::Pin* output_pin = to.AsPointer<VisualMaterial::Pin>();

                if (input_pin->kind() != VisualMaterial::PinKind::Input)
                {
                    std::swap(input_pin, output_pin);
                }

                if (input_pin == output_pin)
                {
                    ed::RejectNewItem();
                }
                else if (input_pin->kind() == output_pin->kind())
                {
                    show_label("editor/Cannot create link to same pin type"_localized, ImColor(255, 0, 0));
                    ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), 3.f);
                }
                else if (input_pin->node() == output_pin->node())
                {
                    show_label("editor/Cannot create link between pins of the same node"_localized, ImColor(255, 0, 0));
                    ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), 3.f);
                }
                else if (!VisualMaterial::is_convertable(input_pin->type(), output_pin->type()))
                {
                    show_label("editor/Incompatible Pin Type"_localized, ImColor(255, 0, 0));
                    ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), 3.f);
                }
                else if (ed::AcceptNewItem())
                {
                    VisualMaterial::InputPin* in   = reinterpret_cast<VisualMaterial::InputPin*>(input_pin);
                    VisualMaterial::OutputPin* out = reinterpret_cast<VisualMaterial::OutputPin*>(output_pin);
                    in->create_link(out);
                }
            }


            if (ed::QueryNewNode(&from))
            {
                VisualMaterial::Pin* pin = from.AsPointer<VisualMaterial::Pin>();
                show_label("editor/+ Create Node"_localized, ImColor(32, 45, 32, 180));

                if (ed::AcceptNewItem() && !ImGui::IsPopupOpen("Create New Node"))
                {
                    ed::Suspend();
                    ImGui::OpenPopup("Create New Node");
                    ed::Resume();

                    out_pin = pin;
                }
            }
        }
        ed::EndCreate();
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
            if (ImGui::MenuItem(instance->base_name().c_str()))
            {
                current = instance;
            }
        }

        return current;
    }

    static bool show_new_node_popup(VisualMaterial* material, VisualMaterial::Pin* from)
    {
        bool status = false;
        ed::Suspend();
        if ((status = ImGui::BeginPopup("Create New Node")))
        {
            ImGui::Dummy({200, 0});

            static Group* root_group = Group::find("Engine::VisualMaterialNodes");
            if (Struct* self = render_node_types(root_group))
            {
                auto node      = material->create_node(self);
                node->position = ImGuiHelpers::construct_vec2<Vector2D>(ed::ScreenToCanvas(ImGui::GetCursorScreenPos()));

                if (from)
                {
                    if (from->kind() == VisualMaterial::PinKind::Input)
                    {
                        for (auto& output : node->outputs())
                        {
                            if (output->type() == from->type())
                            {
                                reinterpret_cast<VisualMaterial::InputPin*>(from)->create_link(output);
                                break;
                            }
                        }

                        for (auto& output : node->outputs())
                        {
                            if (VisualMaterial::is_convertable(output->type(), from->type()))
                            {
                                reinterpret_cast<VisualMaterial::InputPin*>(from)->create_link(output);
                                break;
                            }
                        }
                    }
                    else
                    {
                        for (auto& input : node->inputs())
                        {
                            if (input->type() == from->type())
                            {
                                input->create_link(reinterpret_cast<VisualMaterial::OutputPin*>(from));
                                break;
                            }
                        }

                        for (auto& input : node->inputs())
                        {
                            if (VisualMaterial::is_convertable(input->type(), from->type()))
                            {
                                input->create_link(reinterpret_cast<VisualMaterial::OutputPin*>(from));
                                break;
                            }
                        }
                    }
                }
            }

            ImGui::EndPopup();
        }

        ed::Resume();
        return status;
    }


    static void delete_selected_items()
    {
        size_t objects = ed::GetSelectedObjectCount();
        if (objects == 0)
            return;

        byte* _data = new byte[objects * glm::max(sizeof(ed::NodeId), sizeof(ed::PinId))];

        {
            ed::NodeId* nodes = reinterpret_cast<ed::NodeId*>(_data);
            for (int i = 0, count = ed::GetSelectedNodes(nodes, objects); i < count; i++)
            {
                VisualMaterial::Node* node = nodes[i].AsPointer<VisualMaterial::Node>();

                if (node->is_destroyable())
                {
                    for (VisualMaterial::InputPin* in : node->inputs())
                    {
                        in->unlink();
                    }

                    for (VisualMaterial::OutputPin* out : node->outputs())
                    {
                        out->unlink();
                    }

                    delete node;
                    ed::DeleteNode(nodes[i]);
                }
            }
        }

        {
            ed::LinkId* links = reinterpret_cast<ed::LinkId*>(_data);
            for (int i = 0, count = ed::GetSelectedLinks(links, objects); i < count; i++)
            {
                VisualMaterial::Pin* pin = reinterpret_cast<VisualMaterial::Pin*>(static_cast<Identifier>(links[i]) - 1);

                if (pin)
                {
                    pin->unlink();
                }
            }
        }

        delete[] _data;
        ed::ClearSelection();
    }


    MaterialEditorClient& MaterialEditorClient::render_visual_material_graph(class VisualMaterial* material)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Tab, false))
        {
            ed::Suspend();
            ImGui::OpenPopup("Create New Node");
            ed::Resume();
        }

        render_graph(material->nodes());
        check_creating(m_create_node_from_pin);
        if (!show_new_node_popup(material, reinterpret_cast<VisualMaterial::Pin*>(m_create_node_from_pin)))
        {
            m_create_node_from_pin = nullptr;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            delete_selected_items();
        }
        return *this;
    }



    static PostInitializeController controller([](){
        Object::new_instance_named<VisualMaterial>("TestMaterial")->add_to_package(Object::root_package());
    });
}// namespace Engine
