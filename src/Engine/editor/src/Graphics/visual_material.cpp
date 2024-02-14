#include "Core/struct.hpp"
#include <Core/class.hpp>
#include <Core/flags.hpp>
#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material_nodes.hpp>
#include <Graphics/visual_material.hpp>
#include <imgui_node_editor.h>
#include <theme.hpp>

namespace ed = ax::NodeEditor;

namespace Engine
{
    static Flags<MaterialNodeDataType> one_component_types = Flags<MaterialNodeDataType>() | MaterialNodeDataType::Bool |
                                                             MaterialNodeDataType::Int | MaterialNodeDataType::UInt |
                                                             MaterialNodeDataType::Float;

    static Flags<MaterialNodeDataType> two_component_types = Flags<MaterialNodeDataType>() | MaterialNodeDataType::BVec2 |
                                                             MaterialNodeDataType::IVec2 | MaterialNodeDataType::UVec2 |
                                                             MaterialNodeDataType::Vec2;

    static Flags<MaterialNodeDataType> three_component_types = Flags<MaterialNodeDataType>() | MaterialNodeDataType::BVec3 |
                                                               MaterialNodeDataType::IVec3 | MaterialNodeDataType::UVec3 |
                                                               MaterialNodeDataType::Vec3;

    static Flags<MaterialNodeDataType> four_component_types = Flags<MaterialNodeDataType>() | MaterialNodeDataType::BVec4 |
                                                              MaterialNodeDataType::IVec4 | MaterialNodeDataType::UVec4 |
                                                              MaterialNodeDataType::Vec4;

    static Flags<MaterialNodeDataType> is_boolean_type = Flags<MaterialNodeDataType>() | MaterialNodeDataType::Bool |
                                                         MaterialNodeDataType::BVec2 | MaterialNodeDataType::BVec3 |
                                                         MaterialNodeDataType::BVec4;

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


    static FORCE_INLINE bool is_small_type(Flags<MaterialNodeDataType> type)
    {
        return static_cast<bool>(is_boolean_type & type) ||
               static_cast<MaterialNodeDataType>(type.flags) == MaterialNodeDataType::Color3 ||
               static_cast<MaterialNodeDataType>(type.flags) == MaterialNodeDataType::Color4;
    }

    static FORCE_INLINE float pin_item_len(MaterialPin* pin)
    {
        Flags<MaterialNodeDataType> type = pin->value_type();
        bool is_small                    = is_small_type(type);
        float spacing_multiplier         = is_boolean_type & type ? 2.0 : 1.f;
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


        static Flags<MaterialNodeDataType> is_color =
                Flags<MaterialNodeDataType>() | MaterialNodeDataType::Color3 | MaterialNodeDataType::Color4;

        if ((type & one_component_types) == type)
        {
            return item_width;
        }
        else if ((type & two_component_types) == type)
        {
            return item_width * 2.f + spacing * spacing_multiplier;
        }
        else if ((type & three_component_types) == type)
        {
            return item_width * 3.f + spacing * (2.f * spacing_multiplier);
        }
        else if ((type & four_component_types) == type)
        {
            return item_width * 4.f + spacing * (3.f * spacing_multiplier);
        }
        else if ((type & is_color) == type)
        {
            return item_width;
        }
        return item_width;
    }

    static FORCE_INLINE void render_typed_pin_content(MaterialPin* pin, void* data, int max_len, float max_item_len)
    {
        MaterialNodeDataType type = static_cast<MaterialNodeDataType>(pin->value_type());

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
            case MaterialNodeDataType::Bool:
                render_bool_vector(data, 1);
                break;
            case MaterialNodeDataType::Int:
                ImGui::InputScalarN(item_name, ImGuiDataType_S32, data, 1);
                break;
            case MaterialNodeDataType::UInt:
                ImGui::InputScalarN(item_name, ImGuiDataType_U32, data, 1);
                break;
            case MaterialNodeDataType::Float:
                ImGui::InputScalarN(item_name, ImGuiDataType_Float, data, 1);
                break;

            case MaterialNodeDataType::BVec2:
                render_bool_vector(data, 2);
                break;
            case MaterialNodeDataType::BVec3:
                render_bool_vector(data, 3);
                break;
            case MaterialNodeDataType::BVec4:
                render_bool_vector(data, 4);
                break;

            case MaterialNodeDataType::IVec2:
                ImGui::InputScalarN(item_name, ImGuiDataType_S32, data, 2);
                break;
            case MaterialNodeDataType::IVec3:
                ImGui::InputScalarN(item_name, ImGuiDataType_S32, data, 3);
                break;
            case MaterialNodeDataType::IVec4:
                ImGui::InputScalarN(item_name, ImGuiDataType_S32, data, 4);
                break;
            case MaterialNodeDataType::UVec2:
                ImGui::InputScalarN(item_name, ImGuiDataType_U32, data, 2);
                break;
            case MaterialNodeDataType::UVec3:
                ImGui::InputScalarN(item_name, ImGuiDataType_U32, data, 3);
                break;
            case MaterialNodeDataType::UVec4:
                ImGui::InputScalarN(item_name, ImGuiDataType_U32, data, 4);
                break;
            case MaterialNodeDataType::Vec2:
                ImGui::InputScalarN(item_name, ImGuiDataType_Float, data, 2);
                break;
            case MaterialNodeDataType::Vec3:
                ImGui::InputScalarN(item_name, ImGuiDataType_Float, data, 3);
                break;
            case MaterialNodeDataType::Vec4:
                ImGui::InputScalarN(item_name, ImGuiDataType_Float, data, 4);
                break;
            case MaterialNodeDataType::Color3:
            {
                ImGui::ColorEdit3(item_name, &reinterpret_cast<Vector3D*>(data)->x, ImGuiColorEditFlags_NoInputs);
                break;
            }
            case MaterialNodeDataType::Color4:
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

    static FORCE_INLINE ImVec2 render_pin_content(MaterialPin* pin, float pin_radius, int max_len, float max_item_len)
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

    static FORCE_INLINE void render_pin(MaterialPin* pin, ed::PinKind kind, int max_text_len = 0, float max_item_len = 0.f)
    {
        float radius    = pin_radius();
        float trickness = pin_trickness();

        ImVec2 center = render_pin_content(pin, radius, max_text_len, max_item_len);

        ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 1.0f)), 24);

        ImGui::GetWindowDrawList()->AddCircle(
                center, radius,
                ImGui::GetColorU32(kind == ed::PinKind::Input ? ImVec4(1.f, 0.f, 0.f, 1.0f) : ImVec4(0.f, 1.f, 0.f, 1.0f)), 24,
                trickness);

        ed::BeginPin(pin->id(), kind);
        radius *= 2.f;
        ImVec2 square_size = ImVec2(radius, radius);
        ed::PinRect(center - square_size, center + square_size);

        ed::EndPin();
    }


    static void render_node_header(MaterialNode* node, ImVec2 pos)
    {
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

    static void render_node(MaterialNode* node)
    {
        if (!node)
            return;

        ImVec2 current_position = ed::GetNodePosition(node->id());

        if (current_position.x == FLT_MAX)
        {
            ed::SetNodePosition(node->id(), ImGuiHelpers::construct_vec2<ImVec2>(node->position));
        }
        else
        {
            node->position.x = current_position.x;
            node->position.y = current_position.y;
        }

        ImGui::PushID(node->id());
        ed::BeginNode(node->id());
        {
            auto header_pos = ImGui::GetCursorPos();

            ImGui::BeginGroup();
            {
                ImGui::NewLine();
                ImGui::NewLine();

                ImGui::BeginGroup();
                {
                    int max_len = 0.f;
                    for (MaterialInputPin* pin : node->inputs)
                    {
                        max_len = glm::max(max_len, static_cast<int>(pin->name.to_string().length()));
                    }

                    for (MaterialInputPin* pin : node->inputs)
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
                    for (MaterialOutputPin* pin : node->outputs)
                    {
                        max_len      = glm::max(max_len, static_cast<int>(pin->name.to_string().length()));
                        max_item_len = glm::max(max_item_len, pin_item_len(pin));
                    }

                    for (MaterialOutputPin* pin : node->outputs)
                    {
                        render_pin(pin, ed::PinKind::Output, -max_len, max_item_len);
                    }
                    ImGui::EndGroup();
                }

                ImGui::EndGroup();
            }

            render_node_header(node, header_pos);
            ed::EndNode();
            ImGui::PopID();
        }
    }

    static void render_links(MaterialNode* node)
    {
        for (MaterialInputPin* pin : node->inputs)
        {
            if (pin->linked_to)
            {
                ed::Link(reinterpret_cast<Identifier>(pin) - 1, pin->id(), pin->linked_to->id(), ImColor(0, 149, 220),
                         link_trickess);
            }
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

    static void check_creating_links()
    {
        if (ed::BeginCreate(ImColor(0, 169, 233), link_trickess))
        {
            ed::PinId input, output;
            if (ed::QueryNewLink(&output, &input) && input && output)
            {
                MaterialPin* input_pin  = input.AsPointer<MaterialPin>();
                MaterialPin* output_pin = output.AsPointer<MaterialPin>();

                if (input_pin->type() != MaterialPinType::Input)
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
                    show_link_label("editor/Cannot create link between pins of the same node"_localized, ImColor(255, 0, 0),
                                    ImColor(255, 255, 255, 255));
                    ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), link_trickess);
                }
                else if (ed::AcceptNewItem())
                {
                    MaterialInputPin* in   = reinterpret_cast<MaterialInputPin*>(input_pin);
                    MaterialOutputPin* out = reinterpret_cast<MaterialOutputPin*>(output_pin);

                    if (in->linked_to != out)
                    {
                        if (in->linked_to)
                        {
                            in->linked_to->linked_to.erase(in);
                        }

                        in->linked_to = out;
                        out->linked_to.insert(in);

                        info_log("Material Editor", "Create link from '%s->%s' to %s->%s", out->node->name(),
                                 output_pin->name.c_str(), in->node->name(), input_pin->name.c_str());
                    }
                }
            }
        }
        ed::EndCreate();
    }

    MaterialPin::MaterialPin(MaterialNode* node, Name name) : name(name), node(node)
    {}

    Identifier MaterialPin::id() const
    {
        return reinterpret_cast<Identifier>(this);
    }

    bool MaterialPin::is_input_pin() const
    {
        return type() == MaterialPinType::Input;
    }

    bool MaterialPin::is_output_pin() const
    {
        return type() == MaterialPinType::Output;
    }


    void* MaterialPin::default_value()
    {
        return nullptr;
    }

    MaterialNodeDataType MaterialPin::value_type() const
    {
        return MaterialNodeDataType::Undefined;
    }

    MaterialPin::~MaterialPin()
    {}

    MaterialPinType MaterialInputPin::type() const
    {
        return MaterialPinType::Input;
    }

    MaterialPinType MaterialOutputPin::type() const
    {
        return MaterialPinType::Output;
    }

    const size_t MaterialNode::compile_error = static_cast<size_t>(0);

    size_t MaterialNode::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        return compile_error;
    }

    const char* MaterialNode::name() const
    {
        return "Node";
    }

    MaterialNodeDataType MaterialNode::output_type(const MaterialOutputPin* pin) const
    {
        return MaterialNodeDataType::Undefined;
    }

    Identifier MaterialNode::id() const
    {
        return reinterpret_cast<Identifier>(this);
    }

    Struct* MaterialNode::struct_instance() const
    {
        return nullptr;
    }


    MaterialNode::~MaterialNode()
    {
        for (MaterialPin* pin : inputs)
        {
            delete pin;
        }

        for (MaterialPin* pin : outputs)
        {
            delete pin;
        }

        inputs.clear();
        outputs.clear();
    }

    implement_struct(VertexNode, Engine, );
    implement_struct(FragmentNode, Engine, );

    implement_engine_class_default_init(VisualMaterial);

    MaterialNode* VisualMaterial::vertex_node() const
    {
        return _M_vertex_node;
    }

    MaterialNode* VisualMaterial::fragment_node() const
    {
        return _M_fragment_node;
    }

    const Vector<MaterialNode*>& VisualMaterial::nodes() const
    {
        return _M_nodes;
    }

    MaterialNode* VisualMaterial::create_node(class Struct* node_struct)
    {
        MaterialNode* node = reinterpret_cast<MaterialNode*>(node_struct->create_struct());

        if (node == nullptr)
            return nullptr;

        _M_nodes.push_back(node);
        return node;
    }

    VisualMaterial::VisualMaterial()
    {
        _M_vertex_node               = new MaterialNodes::VertexNode();
        _M_fragment_node             = new MaterialNodes::FragmentNode();
        _M_fragment_node->position.y = 150;

        _M_nodes.push_back(_M_vertex_node);
        _M_nodes.push_back(_M_fragment_node);
    }

    VisualMaterial& VisualMaterial::render_nodes(void* context)
    {
        for (MaterialNode* node : nodes())
        {
            render_node(node);
        }

        for (MaterialNode* node : nodes())
        {
            render_links(node);
        }

        check_creating_links();

        //        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        //        {
        //            delete_selected_items();
        //        }

        return *this;
    }

    VisualMaterial::~VisualMaterial()
    {
        for (auto& ell : _M_nodes)
        {
            delete ell;
        }

        _M_nodes.clear();
    }
}// namespace Engine
