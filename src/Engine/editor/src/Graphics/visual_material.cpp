#include "Core/struct.hpp"
#include <Clients/material_editor_client.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
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
    bool is_equal_types(MaterialNodeDataType type1, MaterialNodeDataType type2)
    {
        if (type1 == type2)
            return true;

        MaterialDataTypeInfo info1 = MaterialDataTypeInfo::from(type1);
        MaterialDataTypeInfo info2 = MaterialDataTypeInfo::from(type2);

        if (info1.components_count != info2.components_count)
            return false;

        return ((info1.base_type == MaterialBaseDataType::Color && info2.base_type == MaterialBaseDataType::Float) ||
                (info1.base_type == MaterialBaseDataType::Float && info2.base_type == MaterialBaseDataType::Color));
    }

    MaterialNodeDataType calculate_operator_types(MaterialNodeDataType& t1, MaterialNodeDataType& t2)
    {
        MaterialDataTypeInfo info1 = MaterialDataTypeInfo::from(t1);
        MaterialDataTypeInfo info2 = MaterialDataTypeInfo::from(t2);

        if (!info1.is_convertable || !info2.is_convertable)
        {
            return t1 == t2 ? t1 : MaterialNodeDataType::Undefined;
        }

        if (info1.is_matrix() || info2.is_matrix())
        {
            if (info1.is_matrix() && info2.is_matrix())
            {
                auto result = info1.components_count < info2.components_count ? t2 : t1;
                t1          = result;
                t2          = result;
                return result;
            }

            if (info1.is_matrix())
            {
                t2 = static_cast<MaterialNodeDataType>(material_type_value(info1.base_type, info1.matrix_size()));
                return t2;
            }
            else
            {
                t1 = static_cast<MaterialNodeDataType>(material_type_value(info2.base_type, info2.matrix_size()));
                return t1;
            }
        }

        auto result = static_cast<size_t>(t1) < static_cast<size_t>(t2) ? t2 : t1;
        t1          = result;
        t2          = result;
        return result;
    }

    MaterialNodeDataType operator_result_between(MaterialNodeDataType t1, MaterialNodeDataType t2)
    {
        return calculate_operator_types(t1, t2);
    }

    MaterialDataTypeInfo MaterialDataTypeInfo::from(MaterialNodeDataType type)
    {
        MaterialDataTypeInfo info;
        size_t value          = static_cast<size_t>(type);
        info.base_type        = static_cast<MaterialBaseDataType>((value >> 1) & 0b111);
        info.components_count = ((value >> 4) & 0b11111);
        info.is_convertable   = static_cast<bool>(value & 0b1);
        return info;
    }

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


    static FORCE_INLINE bool is_small_type(MaterialNodeDataType type)
    {
        MaterialDataTypeInfo info = MaterialDataTypeInfo::from(type);
        return info.base_type == MaterialBaseDataType::Bool || info.base_type == MaterialBaseDataType::Color;
    }

    static FORCE_INLINE float pin_item_len(MaterialPin* pin)
    {
        MaterialNodeDataType type = pin->value_type();
        MaterialDataTypeInfo info = MaterialDataTypeInfo::from(type);
        bool is_small             = is_small_type(type);
        float spacing_multiplier  = info.base_type == MaterialBaseDataType::Bool ? 2.0 : 1.f;
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


        if (info.components_count == 1)
        {
            return item_width;
        }
        else if (info.components_count == 2)
        {
            return item_width * 2.f + spacing * spacing_multiplier;
        }
        else if (info.components_count == 3)
        {
            return item_width * 3.f + spacing * (2.f * spacing_multiplier);
        }
        else if (info.components_count == 4)
        {
            return item_width * 4.f + spacing * (3.f * spacing_multiplier);
        }
        else if (info.base_type == MaterialBaseDataType::Color)
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
                int header_len = static_cast<int>(strlen(node->name()));
                ImGui::Text("%*s", header_len, "");
                ImGui::NewLine();

                header_len /= 2;
                int offset = header_len;

                ImGui::BeginGroup();
                {
                    int max_len = 0.f;
                    for (MaterialInputPin* pin : node->inputs)
                    {
                        max_len = glm::max(max_len, static_cast<int>(pin->name.length()));
                        offset  = glm::max(glm::min(offset - max_len, offset), 0);
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
                        max_len      = glm::max(max_len, static_cast<int>(pin->name.length()));
                        max_item_len = glm::max(max_item_len, pin_item_len(pin));
                    }

                    for (MaterialOutputPin* pin : node->outputs)
                    {
                        if (offset > 0)
                        {
                            ImGui::Text("%*s", offset, "");
                            ImGui::SameLine();
                        }
                        render_pin(pin, ed::PinKind::Output, -max_len, max_item_len);
                    }
                    ImGui::EndGroup();
                }

                node->render();
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
                    in->link_to(out);
                }
            }
        }
        ed::EndCreate();
    }

    static void remove_link(MaterialInputPin* input)
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
        if (objects == 0)
            return;

        byte* _data = new byte[objects * glm::max(sizeof(ed::NodeId), sizeof(ed::PinId))];

        {
            ed::NodeId* nodes = reinterpret_cast<ed::NodeId*>(_data);
            for (int i = 0, count = ed::GetSelectedNodes(nodes, objects); i < count; i++)
            {
                MaterialNode* node = nodes[i].AsPointer<MaterialNode>();

                if (node->is_removable())
                {
                    for (MaterialInputPin* in : node->inputs)
                    {
                        remove_link(in);
                    }

                    for (MaterialOutputPin* out : node->outputs)
                    {
                        while (!out->linked_to.empty())
                        {
                            MaterialInputPin* in = *out->linked_to.begin();
                            remove_link(in);
                        }
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
                MaterialInputPin* pin = reinterpret_cast<MaterialInputPin*>(static_cast<Identifier>(links[i]) + 1);

                if (pin)
                {
                    remove_link(pin);
                }
            }
        }

        delete[] _data;
        ed::ClearSelection();
    }

    static void process_new_selected_node(MaterialEditorClient* client)
    {
        ImGuiObjectProperties* properties = client->properties_window();
        if (!properties)
            return;

        size_t nodes = ed::GetSelectedObjectCount();
        if (nodes != 1)
            return;

        ed::NodeId node_id;
        nodes = ed::GetSelectedNodes(&node_id, 1);

        if (nodes == 1)
        {
            MaterialNode* node = node_id.AsPointer<MaterialNode>();
            if (node == properties->instance())
                return;

            node->bind_to_properties_window(properties);
        }
    }

    MaterialPin::MaterialPin(MaterialNode* node, const String& name) : name(name), node(node)
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

    MaterialNodeDataType MaterialInputPin::value_type() const
    {
        if (linked_to)
        {
            return linked_to->value_type();
        }

        return MaterialNodeDataType::Undefined;
    }

    MaterialPin& MaterialInputPin::link_to(MaterialOutputPin* pin)
    {
        if (pin && ((pin->node == node) || pin == linked_to))
            return *this;

        if (linked_to)
        {
            linked_to->linked_to.erase(this);
        }

        linked_to = pin;

        if (linked_to)
        {
            linked_to->linked_to.insert(this);
        }

        return *this;
    }

    MaterialPinType MaterialOutputPin::type() const
    {
        return MaterialPinType::Output;
    }

    MaterialNodeDataType MaterialOutputPin::value_type() const
    {
        return node->output_type(this);
    }

    size_t MaterialOutputPin::refereces_count() const
    {
        return linked_to.size();
    }

    const size_t MaterialNode::compile_error = static_cast<size_t>(0);

    bool MaterialNode::is_removable() const
    {
        return true;
    }

    void MaterialNode::render()
    {}

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

    void MaterialNode::bind_to_properties_window(ImGuiObjectProperties*)
    {}

    bool MaterialNode::serialize_pins() const
    {
        return true;
    }

    bool MaterialNode::archive_process(Archive& ar)
    {
        if (!SerializableObject::archive_process(ar))
            return false;

        ar & position;

        if (serialize_pins())
        {
            for (MaterialInputPin* pin : inputs)
            {
                pin->archive_process(ar);
            }

            for (MaterialOutputPin* pin : outputs)
            {
                pin->archive_process(ar);
            }
        }
        return true;
    }

    Index MaterialNode::pin_index(MaterialOutputPin* pin) const
    {
        if (pin)
        {
            Index index = 0;
            for (MaterialOutputPin* out : outputs)
            {
                if (out == pin)
                {
                    return index;
                }
                ++index;
            }
        }
        return Constants::index_none;
    }

    Index MaterialNode::pin_index(MaterialInputPin* pin) const
    {
        if (pin)
        {
            Index index = 0;
            for (MaterialInputPin* in : inputs)
            {
                if (in == pin)
                {
                    return index;
                }
                ++index;
            }
        }
        return Constants::index_none;
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

        Index index = 0;
        for (MaterialNode* node : material->m_nodes)
        {
            if (node == this)
            {
                material->m_nodes.erase(material->m_nodes.begin() + index);
                break;
            }
            ++index;
        }
    }

    implement_struct(VertexNode, Engine, );
    implement_struct(FragmentNode, Engine, );

    implement_engine_class_default_init(VisualMaterial);

    MaterialNode* VisualMaterial::vertex_node() const
    {
        return m_vertex_node;
    }

    MaterialNode* VisualMaterial::fragment_node() const
    {
        return m_fragment_node;
    }

    const Vector<MaterialNode*>& VisualMaterial::nodes() const
    {
        return m_nodes;
    }

    Index VisualMaterial::node_index(const MaterialNode* node) const
    {
        if (node)
        {
            Index index = 0;
            for (MaterialNode* material_node : m_nodes)
            {
                if (material_node == node)
                {
                    return index;
                }

                ++index;
            }
        }
        return Constants::index_none;
    }

    MaterialNode* VisualMaterial::create_node(class Struct* node_struct, const Vector2D& position)
    {
        MaterialNode* node = reinterpret_cast<MaterialNode*>(node_struct->create_struct());

        if (node == nullptr)
            return nullptr;

        m_nodes.push_back(node);
        node->material = this;
        node->position = position;
        return node;
    }

    VisualMaterial::VisualMaterial()
    {
        m_vertex_node             = new MaterialNodes::VertexNode();
        m_vertex_node->position.y = -200;
        m_vertex_node->material   = this;

        m_fragment_node             = new MaterialNodes::FragmentNode();
        m_fragment_node->position.y = 200;
        m_fragment_node->material   = this;

        m_nodes.push_back(m_vertex_node);
        m_nodes.push_back(m_fragment_node);
    }

    VisualMaterial& VisualMaterial::render_nodes(MaterialEditorClient* client)
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

        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            delete_selected_items();
        }
        else
        {
            process_new_selected_node(client);
        }

        return *this;
    }

    bool VisualMaterial::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;

        // Serialize base nodes position
        m_vertex_node->archive_process(ar);
        m_fragment_node->archive_process(ar);


        if (ar.is_saving())
        {
            size_t size = m_nodes.size() - 2;
            ar & size;

            for (size_t i = 2, j = m_nodes.size(); i < j; ++i)
            {
                MaterialNode* node = m_nodes[i];
                String node_name   = node->struct_instance()->name().to_string();

                ar & node_name;
                node->archive_process(ar);
            }

            // Serialize links

            for (MaterialNode* node : m_nodes)
            {
                for (MaterialInputPin* pin : node->inputs)
                {
                    Index index     = Constants::index_none;
                    Index pin_index = Constants::index_none;

                    if (pin->linked_to)
                    {
                        index     = node_index(pin->linked_to->node);
                        pin_index = pin->linked_to->node->pin_index(pin->linked_to);
                    }

                    ar & index;
                    ar & pin_index;
                }
            }
        }
        else
        {
            size_t size = 0;
            ar & size;

            while (size > 0)
            {
                String node_name;
                ar & node_name;

                Struct* struct_instance = Struct::static_find(node_name, true);
                MaterialNode* node      = create_node(struct_instance);
                node->archive_process(ar);

                --size;
            }

            // Serialize links

            for (MaterialNode* node : m_nodes)
            {
                for (MaterialInputPin* pin : node->inputs)
                {
                    Index index;
                    Index pin_index;

                    ar & index;
                    ar & pin_index;

                    if (index < m_nodes.size())
                    {
                        MaterialNode* linked_node = m_nodes[index];
                        if (pin_index < linked_node->outputs.size())
                        {
                            pin->link_to(linked_node->outputs[pin_index]);
                        }
                    }
                }
            }
        }
        return ar;
    }

    VisualMaterial::~VisualMaterial()
    {
        while (!m_nodes.empty())
        {
            delete m_nodes.front();
        }
    }
}// namespace Engine
