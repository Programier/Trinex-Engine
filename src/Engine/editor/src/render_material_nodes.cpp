#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/visual_material.hpp>
#include <icons.hpp>
#include <imnodes.h>

namespace Engine
{
    static constexpr float pin_spacing = 10.f;

    static FORCE_INLINE void render_pin_content(NodePin* pin)
    {
        ImGui::Text("%s", pin->name.c_str());
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

    void render_material_nodes(VisualMaterial* material, void* editor_context)
    {
        ImNodes::BeginNodeEditor();

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
    }
}// namespace Engine
