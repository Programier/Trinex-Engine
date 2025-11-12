#include <Core/math/math.hpp>
#include <UI/primitives.hpp>
#include <imgui_internal.h>

namespace Engine::UI
{
	static inline float validate_icon_size(float size)
	{
		return size == 0.f ? size = ImGui::GetTextLineHeightWithSpacing() * 1.5f : size;
	}

	ImVec2 icon_size(float size)
	{
		size = validate_icon_size(size);
		return ImVec2(size, size);
	}

	void plus_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		size = validate_icon_size(size);

		float thickness = size * scale * 0.1;
		ImVec2 center   = pos + ImVec2(size * 0.5f, size * 0.5f);
		float half_len  = size * 0.5f;

		list->AddLine(ImVec2(center.x - half_len, center.y), ImVec2(center.x + half_len, center.y), color, thickness);
		list->AddLine(ImVec2(center.x, center.y - half_len), ImVec2(center.x, center.y + half_len), color, thickness);
	}

	void minus_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		size = validate_icon_size(size);

		float thickness = size * scale * 0.1;
		ImVec2 center   = pos + ImVec2(size * 0.5f, size * 0.5f);
		float half_len  = size * 0.5f;

		list->AddLine(ImVec2(center.x - half_len, center.y), ImVec2(center.x + half_len, center.y), color, thickness);
	}

	void camera_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		size          = validate_icon_size(size);
		ImVec2 center = pos + ImVec2(size * 0.5f, size * 0.5f);

		float body_width  = size * scale;
		float body_height = size * scale * 0.600000f;
		float lens_radius = size * scale * 0.177777f;
		float corner      = size * scale * 0.088888f;
		float thickness   = size * scale * 0.1;

		ImVec2 iconOrigin = ImVec2(center.x - body_width * 0.5f, center.y - body_height * 0.4f);

		list->AddRect(iconOrigin, ImVec2(iconOrigin.x + body_width, iconOrigin.y + body_height), color, corner, 0, thickness);

		ImVec2 lensCenter = ImVec2(iconOrigin.x + body_width * 0.5f, iconOrigin.y + body_height * 0.5f);
		list->AddCircle(lensCenter, lens_radius, color, 24, thickness);

		float viewWidth  = body_width * 0.25f;
		float viewHeight = body_height * 0.25f;
		ImVec2 v0        = ImVec2(iconOrigin.x + body_width * 0.1f, iconOrigin.y - viewHeight * 0.6f);
		ImVec2 v1        = ImVec2(v0.x + viewWidth, v0.y + viewHeight);
		list->AddRect(v0, v1, color, corner * 0.5f, 0, thickness);
	}

	void more_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		size          = validate_icon_size(size);
		ImVec2 center = pos + ImVec2(size * 0.5f, size * 0.5f);

		float radius  = size * scale * 0.1;
		float spacing = size * 0.35f;

		ImVec2 p1 = ImVec2(center.x, center.y - spacing);
		ImVec2 p2 = center;
		ImVec2 p3 = ImVec2(center.x, center.y + spacing);

		list->AddCircleFilled(p1, radius, color, 12);
		list->AddCircleFilled(p2, radius, color, 12);
		list->AddCircleFilled(p3, radius, color, 12);
	}

	void move_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		size          = validate_icon_size(size);
		ImVec2 center = pos + ImVec2(size * 0.5f, size * 0.5f);

		float triangle_size = size * scale * 0.30f;
		float offset        = size * 0.20f;

		list->AddCircleFilled(center, size * scale * 0.1f, color, 12);

		list->AddTriangleFilled(center + ImVec2(0, -offset - triangle_size), center + ImVec2(triangle_size * 0.5f, -offset),
		                        center + ImVec2(-triangle_size * 0.5f, -offset), color);

		list->AddTriangleFilled(center + ImVec2(0, offset + triangle_size), center + ImVec2(-triangle_size * 0.5f, offset),
		                        center + ImVec2(triangle_size * 0.5f, offset), color);

		list->AddTriangleFilled(center + ImVec2(-offset - triangle_size, 0), center + ImVec2(-offset, -triangle_size * 0.5f),
		                        center + ImVec2(-offset, triangle_size * 0.5f), color);

		list->AddTriangleFilled(center + ImVec2(offset + triangle_size, 0), center + ImVec2(offset, triangle_size * 0.5f),
		                        center + ImVec2(offset, -triangle_size * 0.5f), color);
	}

	void rotate_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		size          = validate_icon_size(size);
		ImVec2 center = pos + ImVec2(size * 0.5f, size * 0.5f);

		float radius     = size * 0.4f;
		float arrow_size = size * 0.3f;
		int segments     = 24;

		list->AddCircleFilled(center, size * scale * 0.1, color, 12);

		list->PathArcTo(center, radius, -1.5f * IM_PI, 0.f, segments);
		list->PathStroke(color, false, size * scale * 0.1f);

		ImVec2 p1 = center + ImVec2(radius, arrow_size);
		ImVec2 p2 = p1 + ImVec2(-arrow_size * 0.5f, -arrow_size);
		ImVec2 p3 = p1 + ImVec2(arrow_size * 0.5f, -arrow_size);

		list->AddTriangleFilled(p1, p2, p3, color);
	}

	void scale_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		constexpr float cos_angle = 0.70710678f;

		size          = validate_icon_size(size);
		ImVec2 center = pos + ImVec2(size * 0.5f, size * 0.5f);

		float triangle_size   = size * scale * 0.30f / cos_angle;
		float triangle_offset = size * 0.20f / cos_angle;

		list->AddCircleFilled(center, size * scale * 0.1, color, 12);

		auto draw_triangle = [&](float angle) {
			ImVec2 p1 = ImVec2(triangle_offset + triangle_size * 0.5f, -triangle_offset - triangle_size * 0.5f) * angle;
			ImVec2 p2 = ImVec2(triangle_size * 0.5f + triangle_offset, triangle_size * 0.5f - triangle_offset) * angle;
			ImVec2 p3 = ImVec2(-triangle_size * 0.5f + triangle_offset, -triangle_size * 0.5f - triangle_offset) * angle;
			list->AddTriangleFilled(center + p1, center + p2, center + p3, color);
		};

		draw_triangle(cos_angle);
		draw_triangle(-cos_angle);
	}

	void select_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		size = validate_icon_size(size) * scale;

		const ImVec2 points[] = {
		        pos + ImVec2(size * 0.20f, size * 0.00f), pos + ImVec2(size * 0.85f, size * 0.60f),
		        pos + ImVec2(size * 0.60f, size * 0.60f), pos + ImVec2(size * 0.75f, size * 0.95f),
		        pos + ImVec2(size * 0.55f, size * 1.00f), pos + ImVec2(size * 0.42f, size * 0.70f),
		        pos + ImVec2(size * 0.20f, size * 0.85f),
		};

		list->AddConvexPolyFilled(points, IM_ARRAYSIZE(points), color);
	}

	void globe_icon(ImVec2 pos, float size, float scale)
	{
		ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);
		ImDrawList* list = ImGui::GetWindowDrawList();

		size = validate_icon_size(size);

		ImVec2 center   = pos + ImVec2(size * 0.5f, size * 0.5f);
		float thickness = size * scale * 0.08f;
		float radius    = (size - thickness) * 0.5f;

		list->AddLine(center - ImVec2(radius, 0.f), center + ImVec2(radius, 0.f), color, thickness);
		list->AddLine(center - ImVec2(0.f, radius), center + ImVec2(0.f, radius), color, thickness);
		list->AddEllipse(center, ImVec2(radius / 2.f, radius), color, 0.f, 0, thickness);

		list->AddCircle(center, radius, color, 24, thickness);
	}

	bool icon_button(IconDrawFunc func, const char* str_id, float size, float scale, ImGuiButtonFlags flags)
	{
		const ImGuiStyle& style = ImGui::GetStyle();

		size               = validate_icon_size(size);
		ImVec2 button_size = ImVec2(size, size) + style.FramePadding * 2.f;

		ImGuiID id = ImGui::GetID(str_id);
		ImVec2 pos = ImGui::GetCursorScreenPos();

		const ImRect bb(pos, pos + button_size);

		ImGui::ItemSize(button_size, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

		ImU32 bgColor;
		if (held)
			bgColor = ImGui::GetColorU32(ImGuiCol_ButtonActive);
		else if (hovered)
			bgColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		else
			bgColor = ImGui::GetColorU32(ImGuiCol_Button);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(bb.Min, bb.Max, bgColor, style.FrameRounding);

		func(pos + style.FramePadding, size, scale);

		return pressed;
	}
}// namespace Engine::UI
