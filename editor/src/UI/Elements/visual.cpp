#include <Core/math/math.hpp>
#include <UI/Elements/visual.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	static ImVec2 to_imgui(const Vec2& value)
	{
		return ImVec2(value.x, value.y);
	}

	static ImVec4 to_imgui(const Vec4& value)
	{
		return ImVec4(value.x, value.y, value.z, value.w);
	}

	static ImU32 to_color(const Vec4& value)
	{
		return ImGui::ColorConvertFloat4ToU32(to_imgui(value));
	}

	static ImVec2 add(const ImVec2& lhs, const Vec2& rhs)
	{
		return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
	}

	static ImVec2 draw_pos(const DrawNode* node, const Vec2& pos)
	{
		return add(add(ImGui::GetCursorScreenPos(), node->offset), pos);
	}

	trinex_implement_ui_element(Visual)
	{
		trinex_ui_bind_property(pivot, Style);
		trinex_ui_bind_property(translate, Style);
		trinex_ui_bind_property(scale, Style);
		trinex_ui_bind_property(rotate, Style);
	}

	trinex_implement_ui_element(DrawNode)
	{
		reflection()->bind("offset", &This::offset, Refl::Property::Markup | Refl::Property::Style);
	}

	Visual& Visual::push_style()
	{
		Super::push_style();

		ImGui::BeginTransform({
		        .Translation = translate,
		        .Scale       = scale,
		        .Rotation    = Math::radians(rotate),
		        .Pivot       = pivot,
		});

		return *this;
	}

	Visual& Visual::pop_style()
	{
		ImGui::EndTransform();
		return *Super::pop_style().as<This>();
	}

	trinex_implement_ui_element(Line)
	{
		trinex_ui_bind_property(p1, Markup);
		trinex_ui_bind_property(p2, Markup);
		reflection()->bind("color", &This::color, Refl::Property::Markup | Refl::Property::Style);
		reflection()->bind("thickness", &This::thickness, Refl::Property::Markup | Refl::Property::Style);
	}

	Element::UpdateFlags Line::on_begin_update()
	{
		ImGui::GetWindowDrawList()->AddLine(draw_pos(this, p1), draw_pos(this, p2), to_color(color), thickness);
		return UpdateFlags::Childs;
	}

	trinex_implement_ui_element(Rect)
	{
		trinex_ui_bind_property(min, Markup);
		trinex_ui_bind_property(max, Markup);
		reflection()->bind("color", &This::color, Refl::Property::Markup | Refl::Property::Style);
		reflection()->bind("rounding", &This::rounding, Refl::Property::Markup | Refl::Property::Style);
		reflection()->bind("thickness", &This::thickness, Refl::Property::Markup | Refl::Property::Style);
		trinex_ui_bind_property(flags, Markup);
		trinex_ui_bind_property(filled, Markup);
	}

	Element::UpdateFlags Rect::on_begin_update()
	{
		if (filled)
		{
			ImGui::GetWindowDrawList()->AddRectFilled(draw_pos(this, min), draw_pos(this, max), to_color(color), rounding,
			                                          static_cast<ImDrawFlags>(flags));
		}
		else
		{
			ImGui::GetWindowDrawList()->AddRect(draw_pos(this, min), draw_pos(this, max), to_color(color), rounding,
			                                    static_cast<ImDrawFlags>(flags), thickness);
		}

		return UpdateFlags::Childs;
	}

	trinex_implement_ui_element(Circle)
	{
		trinex_ui_bind_property(center, Markup);
		trinex_ui_bind_property(radius, Markup);
		reflection()->bind("color", &This::color, Refl::Property::Markup | Refl::Property::Style);
		trinex_ui_bind_property(segments, Markup);
		reflection()->bind("thickness", &This::thickness, Refl::Property::Markup | Refl::Property::Style);
		trinex_ui_bind_property(filled, Markup);
	}

	Element::UpdateFlags Circle::on_begin_update()
	{
		if (filled)
		{
			ImGui::GetWindowDrawList()->AddCircleFilled(draw_pos(this, center), radius, to_color(color), segments);
		}
		else
		{
			ImGui::GetWindowDrawList()->AddCircle(draw_pos(this, center), radius, to_color(color), segments, thickness);
		}

		return UpdateFlags::Childs;
	}

	trinex_implement_ui_element(Triangle)
	{
		trinex_ui_bind_property(p1, Markup);
		trinex_ui_bind_property(p2, Markup);
		trinex_ui_bind_property(p3, Markup);
		reflection()->bind("color", &This::color, Refl::Property::Markup | Refl::Property::Style);
		reflection()->bind("thickness", &This::thickness, Refl::Property::Markup | Refl::Property::Style);
		trinex_ui_bind_property(filled, Markup);
	}

	Element::UpdateFlags Triangle::on_begin_update()
	{
		if (filled)
		{
			ImGui::GetWindowDrawList()->AddTriangleFilled(draw_pos(this, p1), draw_pos(this, p2), draw_pos(this, p3),
			                                              to_color(color));
		}
		else
		{
			ImGui::GetWindowDrawList()->AddTriangle(draw_pos(this, p1), draw_pos(this, p2), draw_pos(this, p3), to_color(color),
			                                        thickness);
		}

		return UpdateFlags::Childs;
	}

	trinex_implement_ui_element(Polyline)
	{
		trinex_ui_bind_property(points, Markup);
		reflection()->bind("color", &This::color, Refl::Property::Markup | Refl::Property::Style);
		trinex_ui_bind_property(flags, Markup);
		reflection()->bind("thickness", &This::thickness, Refl::Property::Markup | Refl::Property::Style);
		trinex_ui_bind_property(closed, Markup);
	}

	Element::UpdateFlags Polyline::on_begin_update()
	{
		if (points.size() < 2)
		{
			return UpdateFlags::Childs;
		}

		Vector<ImVec2> resolved;
		resolved.reserve(points.size());
		for (const ImVec2& point : points)
		{
			resolved.push_back(draw_pos(this, Vec2(point.x, point.y)));
		}

		i32 draw_flags = flags;
		if (closed)
		{
			draw_flags |= ImDrawFlags_Closed;
		}

		ImGui::GetWindowDrawList()->AddPolyline(resolved.data(), static_cast<int>(resolved.size()), to_color(color),
		                                        static_cast<ImDrawFlags>(draw_flags), thickness);
		return UpdateFlags::Childs;
	}

	trinex_implement_ui_element(Bezier)
	{
		trinex_ui_bind_property(p1, Markup);
		trinex_ui_bind_property(p2, Markup);
		trinex_ui_bind_property(p3, Markup);
		trinex_ui_bind_property(p4, Markup);
		reflection()->bind("color", &This::color, Refl::Property::Markup | Refl::Property::Style);
		reflection()->bind("thickness", &This::thickness, Refl::Property::Markup | Refl::Property::Style);
		trinex_ui_bind_property(segments, Markup);
	}

	Element::UpdateFlags Bezier::on_begin_update()
	{
		ImGui::GetWindowDrawList()->AddBezierCubic(draw_pos(this, p1), draw_pos(this, p2), draw_pos(this, p3), draw_pos(this, p4),
		                                           to_color(color), thickness, segments);
		return UpdateFlags::Childs;
	}

	trinex_implement_ui_element(DrawText)
	{
		trinex_ui_bind_property(pos, Markup);
		trinex_ui_bind_property(text, Markup);
		reflection()->bind("color", &This::color, Refl::Property::Markup | Refl::Property::Style);
		reflection()->bind("font_size", &This::font_size, Refl::Property::Markup | Refl::Property::Style);
	}

	Element::UpdateFlags DrawText::on_begin_update()
	{
		if (font_size > 0.0f)
		{
			ImGui::GetWindowDrawList()->AddText(nullptr, font_size, draw_pos(this, pos), to_color(color), text.c_str());
		}
		else
		{
			ImGui::GetWindowDrawList()->AddText(draw_pos(this, pos), to_color(color), text.c_str());
		}

		return UpdateFlags::Childs;
	}

	trinex_implement_ui_element(Image)
	{
		trinex_ui_bind_property(texture, Markup);
		trinex_ui_bind_property(sampler, Markup);
		trinex_ui_bind_property(min, Markup);
		trinex_ui_bind_property(max, Markup);
		trinex_ui_bind_property(uv_min, Markup);
		trinex_ui_bind_property(uv_max, Markup);
		reflection()->bind("color", &This::color, Refl::Property::Markup | Refl::Property::Style);
	}

	Element::UpdateFlags Image::on_begin_update()
	{
		if (texture)
		{
			ImGui::GetWindowDrawList()->AddImage(ImTextureID(texture, sampler), draw_pos(this, min), draw_pos(this, max),
			                                     to_imgui(uv_min), to_imgui(uv_max), to_color(color));
		}

		return UpdateFlags::Childs;
	}

	trinex_implement_ui_element(ClipRect)
	{
		trinex_ui_bind_property(min, Markup);
		trinex_ui_bind_property(max, Markup);
		trinex_ui_bind_property(intersect, Markup);
	}

	Element::UpdateFlags ClipRect::on_begin_update()
	{
		ImGui::GetWindowDrawList()->PushClipRect(draw_pos(this, min), draw_pos(this, max), intersect);
		return UpdateFlags::Childs | UpdateFlags::End;
	}

	Element& ClipRect::on_end_update(UpdateFlags flags)
	{
		ImGui::GetWindowDrawList()->PopClipRect();
		return *this;
	}
}// namespace Trinex::UI
