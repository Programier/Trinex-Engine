#include <Clients/material_editor_client.hpp>
#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/visual_material.hpp>
#include <cfloat>
#include <cstring>
#include <icons.hpp>
#include <imgui_node_editor.h>
#include <theme.hpp>

namespace Engine
{
    namespace ed = ax::NodeEditor;

    static constexpr inline EnumerateType one_component_types =
            NodePin::Bool | NodePin::Int | NodePin::UInt | NodePin::Float | NodePin::Color3 | NodePin::Color4;
    static constexpr inline EnumerateType two_component_types = NodePin::BVec2 | NodePin::IVec2 | NodePin::UVec2 | NodePin::Vec2;
    static constexpr inline EnumerateType three_component_types =
            NodePin::BVec3 | NodePin::IVec3 | NodePin::UVec3 | NodePin::Vec3;
    static constexpr inline EnumerateType four_component_types = NodePin::BVec4 | NodePin::IVec4 | NodePin::UVec4 | NodePin::Vec4;

    static constexpr inline EnumerateType is_boolean_type = NodePin::Bool | NodePin::BVec2 | NodePin::BVec3 | NodePin::BVec4;

    static constexpr inline float link_trickess = 4.f;

    static FORCE_INLINE float pin_radius()
    {
        return 5.f * editor_scale_factor();
    }

    static FORCE_INLINE float pin_trickness()
    {
        return 2.f * editor_scale_factor();
    }

    static FORCE_INLINE float pins_content_indent()
    {
        return 10.f * editor_scale_factor();
    }

    static FORCE_INLINE void render_bool_vector(void* _data, int_t count)
    {
        static char id[] = "###id0";
        id[5]            = '0';

        bool* data = reinterpret_cast<bool*>(_data);
        ImGui::Checkbox(id, data);

        for (int_t i = 1; i < count; i++)
        {
            id[5] = '0' + i;
            ImGui::SameLine();
            ImGui::Checkbox(id, data + i);
        }
    }


    static FORCE_INLINE bool is_small_type(EnumerateType type)
    {
        return static_cast<bool>(type & is_boolean_type) || type == NodePin::Color3 || type == NodePin::Color4;
    }

    static FORCE_INLINE float pin_item_len(NodePin* pin)
    {
        EnumerateType type       = pin->data_types;
        bool is_small            = is_small_type(type);
        float spacing_multiplier = type & is_boolean_type ? 2.0 : 1.f;
        float item_width;
        float spacing;

        if (is_small)
        {
            item_width = ImGui::GetFrameHeight();
            spacing    = ImGui::GetStyle().ItemInnerSpacing.x;
        }
        else
        {
            item_width = 100.f;
            spacing    = 0;
        }
        float factor = editor_scale_factor();
        spacing *= factor;
        item_width *= factor;


        if (type & one_component_types)
        {
            return item_width;
        }
        else if (type & two_component_types)
        {
            return item_width * 2.f + spacing * spacing_multiplier;
        }
        else if (type & three_component_types)
        {
            return item_width * 3.f + spacing * (2.f * spacing_multiplier);
        }
        else if (type & four_component_types)
        {
            return item_width * 4.f + spacing * (3.f * spacing_multiplier);
        }
        return item_width;
    }


    static FORCE_INLINE void render_typed_pin_content(NodePin* pin, void* data, int max_len, float max_item_len)
    {
        NodePin::DataType type = static_cast<NodePin::DataType>(pin->data_types);

        ImGui::PushID(pin->name.c_str());

        const char* item_name = nullptr;

        item_name = "###pin_value";

        float item_width = pin_item_len(pin);

        if (pin->is_input_pin())
        {
            ImGui::Text("%*s", max_len, pin->name.c_str());
            ImGui::SameLine();
        }
        else
        {
            ImGui::Dummy({max_item_len - item_width, 0.f});
            ImGui::SameLine();
        }

        ImGui::SetNextItemWidth(item_width);
        switch (type)
        {
            case NodePin::DataType::Bool:
                render_bool_vector(data, 1);
                break;
            case NodePin::DataType::Int:
                ImGui::InputScalar(item_name, ImGuiDataType_S32, data, nullptr, nullptr, nullptr,
                                   ImGuiInputTextFlags_CallbackResize);
                break;
            case NodePin::DataType::UInt:
                ImGui::InputScalar(item_name, ImGuiDataType_U32, data);
                break;
            case NodePin::DataType::Float:
                ImGui::InputScalar(item_name, ImGuiDataType_Float, data);
                break;

            case NodePin::DataType::BVec2:
                render_bool_vector(data, 2);
                break;
            case NodePin::DataType::BVec3:
                render_bool_vector(data, 3);
                break;
            case NodePin::DataType::BVec4:
                render_bool_vector(data, 4);
                break;

            case NodePin::DataType::IVec2:
                ImGui::InputScalarN(item_name, ImGuiDataType_S32, data, 2);
                break;
            case NodePin::DataType::IVec3:
                ImGui::InputScalarN(item_name, ImGuiDataType_S32, data, 3);
                break;
            case NodePin::DataType::IVec4:
                ImGui::InputScalarN(item_name, ImGuiDataType_S32, data, 4);
                break;
            case NodePin::DataType::UVec2:
                ImGui::InputScalarN(item_name, ImGuiDataType_U32, data, 2);
                break;
            case NodePin::DataType::UVec3:
                ImGui::InputScalarN(item_name, ImGuiDataType_U32, data, 3);
                break;
            case NodePin::DataType::UVec4:
                ImGui::InputScalarN(item_name, ImGuiDataType_U32, data, 4);
                break;
            case NodePin::DataType::Vec2:
                ImGui::InputScalarN(item_name, ImGuiDataType_Float, data, 2);
                break;
            case NodePin::DataType::Vec3:
                ImGui::InputScalarN(item_name, ImGuiDataType_Float, data, 3);
                break;
            case NodePin::DataType::Vec4:
                ImGui::InputScalarN(item_name, ImGuiDataType_Float, data, 4);
                break;
            case NodePin::DataType::Color3:
            {
                ImGui::ColorEdit3(item_name, &reinterpret_cast<Vector3D*>(data)->x, ImGuiColorEditFlags_NoInputs);
                break;
            }
            case NodePin::DataType::Color4:
            {
                ImGui::ColorEdit4(item_name, &reinterpret_cast<Vector4D*>(data)->x, ImGuiColorEditFlags_NoInputs);
                break;
            }


            default:
                throw EngineException("Undefined type!");
        }

        if (pin->is_output_pin())
        {
            ImGui::SameLine();
            ImGui::Text("%*s", max_len, pin->name.c_str());
        }


        ImGui::PopID();
    }


    static FORCE_INLINE ImVec2 render_pin_content(NodePin* pin, float pin_radius, int max_len, float max_item_len)
    {
        void* data = pin->default_value();

        ImGui::BeginGroup();

        float offset          = (pin_radius * 2.f);
        ImVec2 pin_center_pos = ImGui::GetCursorPos();

        if (pin->is_input_pin())
        {
            pin_center_pos = ImGui::GetCursorPos();
            pin_center_pos.x += pin_radius;
            ImGui::Dummy({offset, offset});
            ImGui::SameLine();
        }

        if (data)
        {
            render_typed_pin_content(pin, data, max_len, max_item_len);
        }
        else
        {
            ImGui::Text("%*s", max_len, pin->name.c_str());
        }


        if (pin->is_output_pin())
        {
            ImGui::SameLine();
            pin_center_pos = ImGui::GetCursorPos();
            pin_center_pos.x += pin_radius;
            ImGui::Dummy({offset, offset});
        }

        ImGui::EndGroup();
        pin_center_pos.y += ImGui::GetItemRectSize().y / 2.f;
        return pin_center_pos;
    }


    static FORCE_INLINE void render_pin(NodePin* pin, ed::PinKind kind, int max_text_len = 0, float max_item_len = 0.f)
    {
        float radius    = pin_radius();
        float trickness = pin_trickness();

        ImVec2 center = render_pin_content(pin, radius, max_text_len, max_item_len);

        ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 1.0f)), 24);

        ImGui::GetWindowDrawList()->AddCircle(
                center, radius,
                ImGui::GetColorU32(kind == ed::PinKind::Input ? ImVec4(1.f, 0.f, 0.f, 1.0f) : ImVec4(0.f, 1.f, 0.f, 1.0f)), 24,
                trickness);

        ed::BeginPin(pin->id, kind);
        radius += trickness;
        ImVec2 square_size = ImVec2(radius, radius);
        ed::PinRect(center - square_size, center + square_size);

        ed::EndPin();
    }

    static void render_node_header(Node* node, ImVec2 pos)
    {
        // x - left, y - up, z - right, w - down
        auto& style              = ed::GetStyle();
        auto item_min            = ImGui::GetItemRectMin();
        auto item_max            = ImGui::GetItemRectMax();
        const float header_width = item_max.x - item_min.x;

        item_max.y = item_min.y + ImGui::GetTextLineHeightWithSpacing();
        item_max.x += (style.NodePadding.z - style.NodeBorderWidth);
        item_min.x -= (style.NodePadding.x - style.NodeBorderWidth);
        item_min.y -= (style.NodePadding.y - style.NodeBorderWidth);

        // Render header
        ImDrawList* list = ImGui::GetWindowDrawList();
        list->AddRectFilled(item_min, item_max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Header)),
                            style.NodeRounding - style.NodeBorderWidth, ImDrawFlags_RoundCornersTop);

        const char* name = node->name();
        auto name_size   = ImGui::CalcTextSize(name);
        pos.x += (header_width - name_size.x) * 0.5f;
        ImGui::SetCursorPos(pos);
        ImGui::TextUnformatted(name);
    }

    static void render_node(Node* node)
    {
        if (!node)
            return;

        ImVec2 current_position = ed::GetNodePosition(node->id);

        if (current_position.x == FLT_MAX)
        {
            ed::SetNodePosition(node->id, ImGuiHelpers::construct_vec2<ImVec2>(node->position));
        }
        else
        {
            node->position.x = current_position.x;
            node->position.y = current_position.y;
        }

        ed::BeginNode(node->id);
        {
            auto header_pos = ImGui::GetCursorPos();

            ImGui::BeginGroup();
            {
                ImGui::NewLine();
                ImGui::NewLine();

                ImGui::BeginGroup();
                {
                    int max_len = 0.f;
                    for (InputPin* pin : node->input)
                    {
                        max_len = glm::max(max_len, static_cast<int>(pin->name.to_string().length()));
                    }

                    for (InputPin* pin : node->input)
                    {
                        render_pin(pin, ed::PinKind::Input, -max_len);
                    }
                }

                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + pins_content_indent());

                ImGui::BeginGroup();
                {
                    int max_len        = 0.f;
                    float max_item_len = 0.f;
                    for (OutputPin* pin : node->output)
                    {
                        max_len      = glm::max(max_len, static_cast<int>(pin->name.to_string().length()));
                        max_item_len = glm::max(max_item_len, pin_item_len(pin));
                    }

                    for (OutputPin* pin : node->output)
                    {
                        render_pin(pin, ed::PinKind::Output, -max_len, max_item_len);
                    }
                    ImGui::EndGroup();
                }

                ImGui::EndGroup();
            }

            render_node_header(node, header_pos);
            ed::EndNode();
        }
    }

    static void show_link_label(const char* label, ImColor background, ImColor text)
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
        auto size = ImGui::CalcTextSize(label);

        auto padding = ImGui::GetStyle().FramePadding;
        auto spacing = ImGui::GetStyle().ItemSpacing;

        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

        auto rectMin = ImGui::GetCursorScreenPos() - padding;
        auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

        auto drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(rectMin, rectMax, background, size.y * 0.15f);

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(static_cast<ImVec4>(text)));
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
    };

    static void check_creating_links(VisualMaterial* material)
    {
        if (ed::BeginCreate(ImColor(0, 169, 233), link_trickess))
        {
            ed::PinId input, output;
            if (ed::QueryNewLink(&output, &input) && input && output)
            {
                NodePin* input_pin  = input.AsPointer<InputPin>();
                NodePin* output_pin = output.AsPointer<OutputPin>();

                if (input_pin->type() != NodePin::Input)
                {
                    std::swap(input_pin, output_pin);
                }

                if (input_pin == output_pin)
                {
                    ed::RejectNewItem();
                }
                else if (input_pin->type() == output_pin->type())
                {
                    show_link_label("editor/Cannot create link to same pin type"_localized, ImColor(255, 0, 0),
                                    ImColor(255, 255, 255, 255));
                    ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), link_trickess);
                }
                else if (input_pin->node == output_pin->node)
                {
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 15.f * editor_scale_factor());
                    show_link_label("editor/Cannot create link between pins of the same node"_localized, ImColor(255, 0, 0),
                                    ImColor(255, 255, 255, 255));
                    ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), link_trickess);
                }
                else if (ed::AcceptNewItem())
                {
                    InputPin* in   = reinterpret_cast<InputPin*>(input_pin);
                    OutputPin* out = reinterpret_cast<OutputPin*>(output_pin);

                    if (in->linked_to != out)
                    {
                        if (in->linked_to)
                        {
                            in->linked_to->linked_to.erase(in);
                        }

                        in->linked_to = out;
                        out->linked_to.insert(in);

                        info_log("Material Editor", "Create link from '%s->%s' to %s->%s", output_pin->node->name(),
                                 output_pin->name.c_str(), input_pin->node->name(), input_pin->name.c_str());
                    }
                }
            }
        }
        ed::EndCreate();
    }

    static void render_links(Node* node)
    {
        for (InputPin* pin : node->input)
        {
            if (pin->linked_to)
            {
                ed::Link(reinterpret_cast<Identifier>(pin) - 1, pin->id, pin->linked_to->id, ImColor(0, 149, 220), link_trickess);
            }
        }
    }


    static void remove_link(InputPin* input)
    {
        if (input->linked_to)
        {
            input->linked_to->linked_to.erase(input);
            input->linked_to = nullptr;
        }
    }

    static void delete_selected_items()
    {
        size_t objects = ed::GetSelectedObjectCount();
        byte* _data    = new byte[objects * glm::max(sizeof(ed::NodeId), sizeof(ed::PinId))];

        {
            ed::NodeId* nodes = reinterpret_cast<ed::NodeId*>(_data);
            for (int i = 0, count = ed::GetSelectedNodes(nodes, objects); i < count; i++)
            {
                Node* node = nodes[i].AsPointer<Node>();

                if (node->is_removable_element())
                {
                    for (InputPin* in : node->input)
                    {
                        remove_link(in);
                    }

                    for (OutputPin* out : node->output)
                    {
                        while (!out->linked_to.empty())
                        {
                            InputPin* in = *out->linked_to.begin();
                            remove_link(in);
                        }
                    }

                    delete node;
                }
            }
        }

        {
            ed::LinkId* links = reinterpret_cast<ed::LinkId*>(_data);
            for (int i = 0, count = ed::GetSelectedLinks(links, objects); i < count; i++)
            {
                InputPin* pin = reinterpret_cast<InputPin*>(static_cast<Identifier>(links[i]) + 1);
                if (pin)
                {
                    remove_link(pin);
                }
            }
        }

        delete[] _data;
    }

    void render_material_nodes(class MaterialEditorClient* client, void* editor_context)
    {
        ed::EditorContext* context = reinterpret_cast<ed::EditorContext*>(editor_context);
        ed::SetCurrentEditor(context);

        ed::Begin("###Viewport", ImGui::GetContentRegionAvail());

        VisualMaterial* material = client->current_material();

        if (material)
        {
            for (Node* node : material->nodes())
            {
                render_node(node);
            }

            for (Node* node : material->nodes())
            {
                render_links(node);
            }
        }

        check_creating_links(material);

        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            delete_selected_items();
        }

        ed::End();

        ed::SetCurrentEditor(nullptr);
    }
}// namespace Engine
