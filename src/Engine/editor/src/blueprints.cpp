#include <Graphics/texture_2D.hpp>
#include <blueprints.hpp>
#include <editor_resources.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
    using namespace ax::NodeEditor;

    NodeBuilder::NodeBuilder() : m_node_id(0), m_stage(Stage::Invalid), m_has_header(false)
    {}

    bool NodeBuilder::transition_to_stage(Stage stage)
    {
        if (stage == m_stage)
            return false;

        auto old_stage = m_stage;
        m_stage        = stage;

        switch (old_stage)
        {
            case Stage::Begin:
                break;

            case Stage::Header:
                ImGui::EndHorizontal();
                m_header_min = ImGui::GetItemRectMin();
                m_header_max = ImGui::GetItemRectMax();
                ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.y * 2.0f);

                break;

            case Stage::Content:
                break;

            case Stage::Input:
                ed::PopStyleVar(2);

                ImGui::Spring(1, 0);
                ImGui::EndVertical();
                break;

            case Stage::Middle:
                ImGui::EndVertical();
                break;

            case Stage::Output:
                ed::PopStyleVar(2);
                ImGui::Spring(1, 0);
                ImGui::EndVertical();
                break;

            case Stage::End:
                break;

            case Stage::Invalid:
                break;
        }

        switch (stage)
        {
            case Stage::Begin:
                ImGui::BeginVertical("node");
                break;

            case Stage::Header:
                m_has_header = true;

                ImGui::BeginHorizontal("header");
                break;

            case Stage::Content:
                if (old_stage == Stage::Begin)
                    ImGui::Spring(0);

                ImGui::BeginHorizontal("content");
                ImGui::Spring(0, 0);
                break;

            case Stage::Input:
                ImGui::BeginVertical("inputs", ImVec2(0, 0), 0.0f);

                ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(0, 0.5f));
                ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));

                if (!m_has_header)
                    ImGui::Spring(1, 0);
                break;

            case Stage::Middle:
                ImGui::Spring(1);
                ImGui::BeginVertical("middle", ImVec2(0, 0), 1.0f);
                break;

            case Stage::Output:
                if (old_stage == Stage::Middle || old_stage == Stage::Input)
                    ImGui::Spring(1);
                else
                    ImGui::Spring(1, 0);
                ImGui::BeginVertical("outputs", ImVec2(0, 0), 1.0f);

                ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(1.0f, 0.5f));
                ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));

                if (!m_has_header)
                    ImGui::Spring(1, 0);
                break;

            case Stage::End:
                if (old_stage == Stage::Input)
                    ImGui::Spring(1, 0);
                if (old_stage != Stage::Begin)
                    ImGui::EndHorizontal();
                m_content_min = ImGui::GetItemRectMin();
                m_content_max = ImGui::GetItemRectMax();

                ImGui::EndVertical();
                m_node_min = ImGui::GetItemRectMin();
                m_node_max = ImGui::GetItemRectMax();
                break;

            case Stage::Invalid:
                break;
        }

        return true;
    }

    void NodeBuilder::begin(ed::NodeId id)
    {
        m_has_header = false;
        m_header_min = m_header_max = ImVec2();

        ed::PushStyleVar(StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
        ed::BeginNode(id);
        ImGui::PushID(id.AsPointer());
        m_node_id = id;
        transition_to_stage(Stage::Begin);
    }

    void NodeBuilder::end()
    {
        transition_to_stage(Stage::End);

        ed::EndNode();

        if (ImGui::IsItemVisible())
        {
            auto alpha = static_cast<int>(255 * ImGui::GetStyle().Alpha);

            auto draw_list = ed::GetNodeBackgroundDrawList(m_node_id);

            const auto half_border_width = ed::GetStyle().NodeBorderWidth * 0.5f;

            auto header_color = IM_COL32(0, 0, 0, alpha) | (m_header_color & IM_COL32(255, 255, 255, 0));
            if ((m_header_max.x > m_header_min.x) && (m_header_max.y > m_header_min.y))
            {
                Texture2D* texture = EditorResources::blueprint_texture;
                const auto uv      = ImVec2((m_header_max.x - m_header_min.x) / (4.0f * texture->size.x),
                                            (m_header_max.y - m_header_min.y) / (4.0f * texture->size.y));

                draw_list->AddImageRounded(ImTextureID(texture, EditorResources::default_sampler),
                                           m_header_min - ImVec2(8 - half_border_width, 4 - half_border_width),
                                           m_header_max + ImVec2(8 - half_border_width, 0), ImVec2(0.0f, 0.0f), uv, header_color,
                                           GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);


                if (m_content_min.y > m_header_max.y)
                {
                    draw_list->AddLine(ImVec2(m_header_min.x - (8 - half_border_width), m_header_max.y - 0.5f),
                                       ImVec2(m_header_max.x + (8 - half_border_width), m_header_max.y - 0.5f),
                                       ImColor(255, 255, 255, 96 * alpha / (3 * 255)), 1.0f);
                }
            }
        }

        m_node_id = 0;
        ImGui::PopID();
        ed::PopStyleVar();
        transition_to_stage(Stage::Invalid);
    }

    void NodeBuilder::begin_header(const ImVec4& color)
    {
        m_header_color = ImColor(color);
        transition_to_stage(Stage::Header);
    }

    void NodeBuilder::end_header()
    {
        transition_to_stage(Stage::Content);
    }

    void NodeBuilder::begin_input(ed::PinId id)
    {
        if (m_stage == Stage::Begin)
            transition_to_stage(Stage::Content);

        const auto applyPadding = (m_stage == Stage::Input);

        transition_to_stage(Stage::Input);

        if (applyPadding)
            ImGui::Spring(0);

        begin_pin(id, PinKind::Input);

        ImGui::BeginHorizontal(id.AsPointer());
    }

    void NodeBuilder::end_input()
    {
        ImGui::EndHorizontal();
        EndPin();
    }

    void NodeBuilder::middle()
    {
        if (m_stage == Stage::Begin)
            transition_to_stage(Stage::Content);

        transition_to_stage(Stage::Middle);
    }

    void NodeBuilder::begin_output(ed::PinId id)
    {
        if (m_stage == Stage::Begin)
            transition_to_stage(Stage::Content);

        const auto apply_padding = (m_stage == Stage::Output);

        transition_to_stage(Stage::Output);

        if (apply_padding)
            ImGui::Spring(0);

        begin_pin(id, PinKind::Output);
        ImGui::BeginHorizontal(id.AsPointer());
    }

    void NodeBuilder::end_output()
    {
        ImGui::EndHorizontal();
        end_pin();
    }


    void NodeBuilder::begin_pin(ed::PinId id, ed::PinKind kind)
    {
        ed::BeginPin(id, kind);
    }

    void NodeBuilder::end_pin()
    {
        ed::EndPin();
    }


    static void draw_icon(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max, NodeBuilder::IconType type, bool filled,
                          ImU32 color, ImU32 inner_color)
    {
        auto rect                 = ImRect(min, max);
        auto rect_x               = rect.Min.x;
        auto rect_y               = rect.Min.y;
        auto rect_w               = rect.Max.x - rect.Min.x;
        auto rect_h               = rect.Max.y - rect.Min.y;
        auto rect_center_x        = (rect.Min.x + rect.Max.x) * 0.5f;
        auto rect_center_y        = (rect.Min.y + rect.Max.y) * 0.5f;
        auto rect_center          = ImVec2(rect_center_x, rect_center_y);
        const auto outline_scale  = rect_w / 24.0f;
        const auto extra_segments = static_cast<int>(2 * outline_scale);// for full circle

        if (type == NodeBuilder::IconType::Flow)
        {
            const auto origin_scale = rect_w / 24.0f;

            const auto offset_x  = 1.0f * origin_scale;
            const auto offset_y  = 0.0f * origin_scale;
            const auto margin    = (filled ? 2.0f : 2.0f) * origin_scale;
            const auto rounding  = 0.1f * origin_scale;
            const auto tip_round = 0.7f;
            const auto canvas    = ImRect(rect.Min.x + margin + offset_x, rect.Min.y + margin + offset_y,
                                          rect.Max.x - margin + offset_x, rect.Max.y - margin + offset_y);
            const auto canvas_x  = canvas.Min.x;
            const auto canvas_y  = canvas.Min.y;
            const auto canvas_w  = canvas.Max.x - canvas.Min.x;
            const auto canvas_h  = canvas.Max.y - canvas.Min.y;

            const auto left     = canvas_x + canvas_w * 0.5f * 0.3f;
            const auto right    = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
            const auto top      = canvas_y + canvas_h * 0.5f * 0.2f;
            const auto bottom   = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
            const auto center_y = (top + bottom) * 0.5f;

            const auto tip_top    = ImVec2(canvas_x + canvas_w * 0.5f, top);
            const auto tip_right  = ImVec2(right, center_y);
            const auto tip_bottom = ImVec2(canvas_x + canvas_w * 0.5f, bottom);

            draw_list->PathLineTo(ImVec2(left, top) + ImVec2(0, rounding));
            draw_list->PathBezierCubicCurveTo(ImVec2(left, top), ImVec2(left, top), ImVec2(left, top) + ImVec2(rounding, 0));
            draw_list->PathLineTo(tip_top);
            draw_list->PathLineTo(tip_top + (tip_right - tip_top) * tip_round);
            draw_list->PathBezierCubicCurveTo(tip_right, tip_right, tip_bottom + (tip_right - tip_bottom) * tip_round);
            draw_list->PathLineTo(tip_bottom);
            draw_list->PathLineTo(ImVec2(left, bottom) + ImVec2(rounding, 0));
            draw_list->PathBezierCubicCurveTo(ImVec2(left, bottom), ImVec2(left, bottom),
                                              ImVec2(left, bottom) - ImVec2(0, rounding));

            if (!filled)
            {
                if (inner_color & 0xFF000000)
                    draw_list->AddConvexPolyFilled(draw_list->_Path.Data, draw_list->_Path.Size, inner_color);

                draw_list->PathStroke(color, true, 2.0f * outline_scale);
            }
            else
                draw_list->PathFillConvex(color);
        }
        else
        {
            auto triangleStart = rect_center_x + 0.32f * rect_w;

            auto rect_offset = -static_cast<int>(rect_w * 0.25f * 0.25f);

            rect.Min.x += rect_offset;
            rect.Max.x += rect_offset;
            rect_x += rect_offset;
            rect_center_x += rect_offset * 0.5f;
            rect_center.x += rect_offset * 0.5f;

            if (type == NodeBuilder::IconType::Circle)
            {
                const auto c = rect_center;

                if (!filled)
                {
                    const auto r = 0.5f * rect_w / 2.0f - 0.5f;

                    if (inner_color & 0xFF000000)
                        draw_list->AddCircleFilled(c, r, inner_color, 12 + extra_segments);
                    draw_list->AddCircle(c, r, color, 12 + extra_segments, 2.0f * outline_scale);
                }
                else
                {
                    draw_list->AddCircleFilled(c, 0.5f * rect_w / 2.0f, color, 12 + extra_segments);
                }
            }

            if (type == NodeBuilder::IconType::Square)
            {
                if (filled)
                {
                    const auto r  = 0.5f * rect_w / 2.0f;
                    const auto p0 = rect_center - ImVec2(r, r);
                    const auto p1 = rect_center + ImVec2(r, r);
                    draw_list->AddRectFilled(p0, p1, color, 0, ImDrawFlags_RoundCornersAll);
                }
                else
                {
                    const auto r  = 0.5f * rect_w / 2.0f - 0.5f;
                    const auto p0 = rect_center - ImVec2(r, r);
                    const auto p1 = rect_center + ImVec2(r, r);

                    if (inner_color & 0xFF000000)
                    {
                        draw_list->AddRectFilled(p0, p1, inner_color, 0, ImDrawFlags_RoundCornersAll);
                    }
                    draw_list->AddRect(p0, p1, color, 0, ImDrawFlags_RoundCornersAll, 2.0f * outline_scale);
                }
            }

            if (type == NodeBuilder::IconType::Grid)
            {
                const auto r = 0.5f * rect_w / 2.0f;
                const auto w = ceilf(r / 3.0f);

                const auto baseTl = ImVec2(floorf(rect_center_x - w * 2.5f), floorf(rect_center_y - w * 2.5f));
                const auto baseBr = ImVec2(floorf(baseTl.x + w), floorf(baseTl.y + w));

                auto tl = baseTl;
                auto br = baseBr;
                for (int i = 0; i < 3; ++i)
                {
                    tl.x = baseTl.x;
                    br.x = baseBr.x;
                    draw_list->AddRectFilled(tl, br, color);
                    tl.x += w * 2;
                    br.x += w * 2;
                    if (i != 1 || filled)
                        draw_list->AddRectFilled(tl, br, color);
                    tl.x += w * 2;
                    br.x += w * 2;
                    draw_list->AddRectFilled(tl, br, color);

                    tl.y += w * 2;
                    br.y += w * 2;
                }

                triangleStart = br.x + w + 1.0f / 24.0f * rect_w;
            }

            if (type == NodeBuilder::IconType::RoundSquare)
            {
                if (filled)
                {
                    const auto r  = 0.5f * rect_w / 2.0f;
                    const auto cr = r * 0.5f;
                    const auto p0 = rect_center - ImVec2(r, r);
                    const auto p1 = rect_center + ImVec2(r, r);
                    draw_list->AddRectFilled(p0, p1, color, cr, ImDrawFlags_RoundCornersAll);
                }
                else
                {
                    const auto r  = 0.5f * rect_w / 2.0f - 0.5f;
                    const auto cr = r * 0.5f;
                    const auto p0 = rect_center - ImVec2(r, r);
                    const auto p1 = rect_center + ImVec2(r, r);

                    if (inner_color & 0xFF000000)
                    {
                        draw_list->AddRectFilled(p0, p1, inner_color, cr, ImDrawFlags_RoundCornersAll);
                    }

                    draw_list->AddRect(p0, p1, color, cr, ImDrawFlags_RoundCornersAll, 2.0f * outline_scale);
                }
            }
            else if (type == NodeBuilder::IconType::Diamond)
            {
                if (filled)
                {
                    const auto r = 0.607f * rect_w / 2.0f;
                    const auto c = rect_center;

                    draw_list->PathLineTo(c + ImVec2(0, -r));
                    draw_list->PathLineTo(c + ImVec2(r, 0));
                    draw_list->PathLineTo(c + ImVec2(0, r));
                    draw_list->PathLineTo(c + ImVec2(-r, 0));
                    draw_list->PathFillConvex(color);
                }
                else
                {
                    const auto r = 0.607f * rect_w / 2.0f - 0.5f;
                    const auto c = rect_center;

                    draw_list->PathLineTo(c + ImVec2(0, -r));
                    draw_list->PathLineTo(c + ImVec2(r, 0));
                    draw_list->PathLineTo(c + ImVec2(0, r));
                    draw_list->PathLineTo(c + ImVec2(-r, 0));

                    if (inner_color & 0xFF000000)
                        draw_list->AddConvexPolyFilled(draw_list->_Path.Data, draw_list->_Path.Size, inner_color);

                    draw_list->PathStroke(color, true, 2.0f * outline_scale);
                }
            }
            else
            {
                const auto triangleTip = triangleStart + rect_w * (0.45f - 0.32f);

                draw_list->AddTriangleFilled(ImVec2(ceilf(triangleTip), rect_y + rect_h * 0.5f),
                                             ImVec2(triangleStart, rect_center_y + 0.15f * rect_h),
                                             ImVec2(triangleStart, rect_center_y - 0.15f * rect_h), color);
            }
        }
    }

    void NodeBuilder::icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color, const ImVec4& inner_color)
    {
        if (ImGui::IsRectVisible(size))
        {
            auto cursorPos = ImGui::GetCursorScreenPos();
            auto draw_list = ImGui::GetWindowDrawList();
            draw_icon(draw_list, cursorPos, cursorPos + size, type, filled, ImColor(color), ImColor(inner_color));
        }

        ImGui::Dummy(size);
    }

}// namespace Engine
