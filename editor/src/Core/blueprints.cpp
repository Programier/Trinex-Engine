#include <Core/blueprints.hpp>
#include <Core/editor_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Graphics/texture_2D.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	using namespace ax::NodeEditor;

	BlueprintBuilder::BlueprintBuilder() : m_stage(Stage::Invalid), m_has_header(false), m_require_spacing(false) {}

	void BlueprintBuilder::transition_to_stage(Stage new_stage)
	{
		if (m_stage == new_stage)
			return;

		Stage old = m_stage;
		m_stage   = new_stage;

		switch (old)
		{
			case Stage::Invalid:
				break;
			case Stage::Begin:
				break;
			case Stage::End:
				break;
			case Stage::Header:
				ImGui::EndHorizontal();
				m_header_min = ImGui::GetItemRectMin();
				m_header_max = ImGui::GetItemRectMax();
				break;
			case Stage::Content:
				break;

			case Stage::Input:
				ed::PopStyleVar(2);
				ImGui::EndVertical();
				break;

			case Stage::Middle:
				ImGui::Spring(0.f, 0.5f);
				ImGui::EndVertical();
				break;

			case Stage::Output:
				ed::PopStyleVar(2);
				ImGui::EndVertical();
				break;

			case Stage::Footer:
				ImGui::EndHorizontal();
				m_footer_min = ImGui::GetItemRectMin();
				m_footer_max = ImGui::GetItemRectMax();
				break;

			default:
				break;
		}

		if (m_require_spacing && is_in<Stage::Middle, Stage::Output>(new_stage))
		{
			ImGui::Dummy({ImGui::GetTextLineHeightWithSpacing(), 0.f});
			m_require_spacing = false;
		}

		switch (new_stage)
		{
			case Stage::Invalid:
				break;
			case Stage::Begin:
				ImGui::BeginVertical("Node");
				ImGui::Spring(0.f, 0.f);
				break;
			case Stage::End:
				if (old != Stage::Begin && old != Stage::Footer)
				{
					ImGui::Spring(0.f, 1.f);
					ImGui::EndHorizontal();
				}
				m_content_min = ImGui::GetItemRectMin();
				m_content_max = ImGui::GetItemRectMax();

				ImGui::Spring(0.f, 1.f);
				ImGui::EndVertical();
				m_node_min = ImGui::GetItemRectMin();
				m_node_max = ImGui::GetItemRectMax();
				break;
			case Stage::Header:
				m_has_header = true;
				ImGui::BeginHorizontal("Header");
				break;
			case Stage::Content:
				ImGui::BeginHorizontal("Content");
				ImGui::Spring(0.f, 0.f);
				break;

			case Stage::Input:
				ImGui::BeginVertical("Inputs");
				ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(0, 0.5f));
				ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));
				break;

			case Stage::Middle:
				if (old == Stage::Input)
					ImGui::Spring(1.f, 0.5f);
				ImGui::BeginVertical("Middle");
				ImGui::Spring(0.f, 0.5f);
				break;

			case Stage::Output:
				ImGui::Spring(1.f, 1.f);
				ImGui::BeginVertical("Outputs");
				ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(0, 0.5f));
				ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));
				break;

			case Stage::Footer:
				if (old != Stage::Begin)
					ImGui::EndHorizontal();
				ImGui::BeginHorizontal("Footer");
				m_has_footer = true;
				break;

			default:
				break;
		}
	}

	void BlueprintBuilder::begin(Identifier id)
	{
		m_id         = id;
		m_footer_min = m_footer_max = m_header_min = m_header_max = ImVec2();

		m_has_header      = false;
		m_has_footer      = false;
		m_require_spacing = false;

		//ed::PushStyleVar(StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
		ed::BeginNode(id);
		ImGui::PushID(id);
		transition_to_stage(Stage::Begin);
	}

	void BlueprintBuilder::end()
	{
		transition_to_stage(Stage::End);
		ImGui::PopID();
		ed::EndNode();

		if (ImGui::IsItemVisible())
		{
			auto alpha     = static_cast<int>(255 * ImGui::GetStyle().Alpha);
			auto draw_list = ed::GetNodeBackgroundDrawList(m_id);

			const auto half_border_width = ed::GetStyle().NodeBorderWidth * 0.5f;
			const auto node_padding      = ed::GetStyle().NodePadding;

			if (m_has_header)
			{
				auto header_color = IM_COL32(0, 0, 0, alpha) | (m_header_color & IM_COL32(255, 255, 255, 0));
				if ((m_header_max.x > m_header_min.x) && (m_header_max.y > m_header_min.y))
				{
					Texture2D* texture = EditorResources::blueprint_texture;
					const auto uv      = ImVec2((m_header_max.x - m_header_min.x) / (4.0f * texture->width()),
					                            (m_header_max.y - m_header_min.y) / (4.0f * texture->height()));

					draw_list->AddImageRounded(
					        texture,
					        m_header_min - ImVec2(node_padding.x - half_border_width, node_padding.y - half_border_width),
					        m_header_max + ImVec2(node_padding.z - half_border_width, 0), ImVec2(0.0f, 0.0f), uv, header_color,
					        GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);


					if (m_content_min.y > m_header_max.y)
					{
						draw_list->AddLine(ImVec2(m_header_min.x - (8 - half_border_width), m_header_max.y - 0.5f),
						                   ImVec2(m_header_max.x + (8 - half_border_width), m_header_max.y - 0.5f),
						                   ImColor(255, 255, 255, 96 * alpha / (3 * 255)), 1.0f);
					}
				}
			}

			if (m_has_footer)
			{
				auto footer_color = IM_COL32(0, 0, 0, alpha) | (m_footer_color & IM_COL32(255, 255, 255, 0));
				if ((m_footer_max.x > m_footer_min.x) && (m_footer_max.y > m_footer_min.y))
				{
					Texture2D* texture = EditorResources::blueprint_texture;
					const auto uv      = ImVec2((m_footer_max.x - m_footer_min.x) / (4.0f * texture->width()),
					                            (m_footer_max.y - m_footer_min.y) / (4.0f * texture->height()));

					draw_list->AddImageRounded(
					        texture, m_footer_min - ImVec2(node_padding.x - half_border_width, 0),
					        m_footer_max + ImVec2(node_padding.z - half_border_width, node_padding.w - half_border_width),
					        ImVec2(0.0f, 0.0f), uv, footer_color, GetStyle().NodeRounding, ImDrawFlags_RoundCornersBottom);
				}
			}
		}

		m_id = 0;
		transition_to_stage(Stage::Invalid);
	}

	void BlueprintBuilder::begin_header(const ImVec4& color)
	{
		m_header_color = ImColor(color);
		transition_to_stage(Stage::Header);
	}

	void BlueprintBuilder::end_header()
	{
		transition_to_stage(Stage::Content);
	}

	void BlueprintBuilder::begin_input(Identifier id)
	{
		if (m_stage == Stage::Begin)
			transition_to_stage(Stage::Content);

		transition_to_stage(Stage::Input);
		ImGui::BeginHorizontal(id);
		m_require_spacing = true;
	}

	void BlueprintBuilder::begin_input_pin(Identifier id)
	{
		ed::BeginPin(id, ed::PinKind::Input);
	}

	void BlueprintBuilder::end_input_pin()
	{
		ed::EndPin();
	}

	void BlueprintBuilder::end_input()
	{
		ImGui::EndHorizontal();
	}

	void BlueprintBuilder::begin_middle()
	{
		if (m_stage == Stage::Begin)
			transition_to_stage(Stage::Content);

		transition_to_stage(Stage::Middle);
		m_require_spacing = true;
	}

	void BlueprintBuilder::begin_output(Identifier id)
	{
		if (m_stage == Stage::Begin)
			transition_to_stage(Stage::Content);

		transition_to_stage(Stage::Output);
		ImGui::BeginHorizontal(id);
	}

	void BlueprintBuilder::begin_output_pin(Identifier id)
	{
		ed::BeginPin(id, ed::PinKind::Output);
	}

	void BlueprintBuilder::end_output_pin()
	{
		ed::EndPin();
	}

	void BlueprintBuilder::end_output()
	{
		ImGui::EndHorizontal();
	}

	void BlueprintBuilder::begin_footer(const ImVec4& color)
	{
		transition_to_stage(Stage::Footer);
		m_footer_color = ImColor(color);
	}

	static void draw_icon(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max, BlueprintBuilder::IconType type,
	                      bool filled, ImU32 color, ImU32 inner_color)
	{
		auto rect                 = ImRect(min, max);
		auto rect_y               = rect.Min.y;
		auto rect_w               = rect.Max.x - rect.Min.x;
		auto rect_h               = rect.Max.y - rect.Min.y;
		auto rect_center_x        = (rect.Min.x + rect.Max.x) * 0.5f;
		auto rect_center_y        = (rect.Min.y + rect.Max.y) * 0.5f;
		auto rect_center          = ImVec2(rect_center_x, rect_center_y);
		const auto outline_scale  = rect_w / 24.0f;
		const auto extra_segments = static_cast<int>(2 * outline_scale);// for full circle

		if (type == BlueprintBuilder::IconType::Flow)
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
			rect_center_x += rect_offset * 0.5f;
			rect_center.x += rect_offset * 0.5f;

			if (type == BlueprintBuilder::IconType::Circle)
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

			if (type == BlueprintBuilder::IconType::Square)
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

			if (type == BlueprintBuilder::IconType::Grid)
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

			if (type == BlueprintBuilder::IconType::RoundSquare)
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
			else if (type == BlueprintBuilder::IconType::Diamond)
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

	void BlueprintBuilder::icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color, const ImVec4& inner_color)
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
