#include <Clients/material_editor_client.hpp>
#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/visual_material.hpp>
#include <icons.hpp>
#include <imnodes.h>
#include <theme.hpp>

namespace Engine
{
    static constexpr float pin_spacing = 10.f;

    static FORCE_INLINE void render_typed_pin_content(NodePin* pin, void* data)
    {
        NodePin::DataType type = static_cast<NodePin::DataType>(pin->data_type);

        float item_width = 100.f * editor_scale_factor();


        ImGui::PushID(pin->name.c_str());

        const char* item_name = nullptr;

        if (pin->is_input_pin())
        {
            ImGui::Text("%s", pin->name.c_str());
            ImGui::SameLine();
            item_name = "###pin_value";
        }
        else
        {
            item_name = pin->name.c_str();
        }

        ImGui::SetNextItemWidth(item_width);

        switch (type)
        {
            case NodePin::DataType::Bool:
                ImGui::Checkbox(item_name, reinterpret_cast<bool*>(data));
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
                break;
            case NodePin::DataType::BVec3:
                break;
            case NodePin::DataType::BVec4:
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
            case NodePin::DataType::Color:
                ImGui::ColorEdit4(item_name, &reinterpret_cast<Vector4D*>(data)->x, ImGuiColorEditFlags_NoInputs);
                break;

            default:
                throw EngineException("Undefined type!");
        }


        ImGui::PopID();
    }

    static FORCE_INLINE void render_pin_content(NodePin* pin)
    {
        void* data = pin->default_value();
        if (data)
        {
            render_typed_pin_content(pin, data);
        }
        else
        {
            ImGui::Text("%s", pin->name.c_str());
        }
    }

    static FORCE_INLINE void render_output_pin(OutputPin* output)
    {
        ImNodes::BeginOutputAttribute(output->id, ImNodesPinShape_TriangleFilled);
        render_pin_content(output);
        ImNodes::EndOutputAttribute();
    }

    static FORCE_INLINE void render_input_pin(InputPin* input)
    {
        ImNodes::BeginInputAttribute(input->id, ImNodesPinShape_CircleFilled);
        render_pin_content(input);
        ImNodes::EndInputAttribute();
    }

    static void render_node(Node* node)
    {
        if (!node)
            return;


        ImNodes::BeginNode(node->id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(node->name());
        ImNodes::EndNodeTitleBar();

        ImGui::BeginGroup();
        for (InputPin* pin : node->input)
        {
            render_input_pin(pin);
        }

        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::Dummy({pin_spacing, 1.f});
        ImGui::SameLine();
        ImGui::BeginGroup();

        for (OutputPin* pin : node->output)
        {
            render_output_pin(pin);
        }

        ImGui::EndGroup();

        ImNodes::EndNode();
    }

    static void create_new_link(VisualMaterial* material, Identifier output_pin, Identifier input_pin)
    {
        InputPin* input   = nullptr;
        OutputPin* output = nullptr;

        for (Node* node : material->nodes())
        {
            if (!input)
            {
                for (InputPin* pin : node->input)
                {
                    if (pin->id == input_pin)
                    {
                        input = pin;
                        break;
                    }
                }
            }

            if (!output)
            {
                for (OutputPin* pin : node->output)
                {
                    if (pin->id == output_pin)
                    {
                        output = pin;
                        break;
                    }
                }
            }

            if (input && output)
            {
                input->linked_to = output;
                return;
            }
        }

        error_log("Material Editor", "Failed to link nodes!");
    }

    static void check_creating_links(VisualMaterial* material)
    {
        int start_attr, end_attr;
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
        {
            create_new_link(material, static_cast<Identifier>(start_attr), static_cast<Identifier>(end_attr));
        }
    }

    static void render_links(Node* node)
    {
        for (InputPin* pin : node->input)
        {
            if (pin->linked_to)
            {
                ImNodes::Link(pin->id, pin->id, pin->linked_to->id);
            }
        }
    }

    void render_material_nodes(class MaterialEditorClient* client, void* editor_context)
    {
        ImNodes::SetCurrentContext(static_cast<ImNodesContext*>(editor_context));
        ImNodes::BeginNodeEditor();

        VisualMaterial* material = client->current_material();

        if (material)
        {
            // Render nodes
            for (Node* node : material->nodes())
            {
                render_node(node);
            }

            // Render links
            for (Node* node : material->nodes())
            {
                render_links(node);
            }
        }

        ImNodes::MiniMap(0.15, ImNodesMiniMapLocation_TopLeft);
        ImNodes::EndNodeEditor();

        if (material)
        {
            check_creating_links(material);
        }

        ImNodes::SetCurrentContext(nullptr);
    }
}// namespace Engine
