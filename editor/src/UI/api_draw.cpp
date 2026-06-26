#include "api_internal.hpp"
#include <UI/types.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	static inline DrawListHandle* convert(ImDrawList* list)
	{
		return reinterpret_cast<DrawListHandle*>(list);
	}

	static inline ImDrawList* convert(DrawListHandle* list)
	{
		return reinterpret_cast<ImDrawList*>(list);
	}

	static inline ImTextureRef convert(const Texture& texture)
	{
		return ImTextureID(texture.texture, texture.sampler);
	}

	DrawListHandle* draw_list(DrawList list)
	{
		return convert(resolve_draw_list(list, ImGui::GetCurrentWindow()));
	}

	DrawListHandle& DrawListHandle::push_clip(Vec2 min, Vec2 max, bool intersect_with_current_clip_rect)
	{
		convert(this)->PushClipRect(to_imvec(min), to_imvec(max), intersect_with_current_clip_rect);
		return *this;
	}

	DrawListHandle& DrawListHandle::push_fullscreen_clip()
	{
		convert(this)->PushClipRectFullScreen();
		return *this;
	}

	DrawListHandle& DrawListHandle::pop_clip()
	{
		convert(this)->PopClipRect();
		return *this;
	}

	DrawListHandle& DrawListHandle::push_texture(const Texture& texture)
	{
		convert(this)->PushTexture(convert(texture));
		return *this;
	}

	DrawListHandle& DrawListHandle::pop_texture()
	{
		convert(this)->PopTexture();
		return *this;
	}

	Vec2 DrawListHandle::clip_min()
	{
		return to_vec(convert(this)->GetClipRectMin());
	}

	Vec2 DrawListHandle::clip_max()
	{
		return to_vec(convert(this)->GetClipRectMax());
	}

	DrawListHandle& DrawListHandle::line(Vec2 p1, Vec2 p2, u32 col, f32 thickness)
	{
		convert(this)->AddLine(to_imvec(p1), to_imvec(p2), col, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::rect(Vec2 min, Vec2 max, u32 col, f32 rounding, DrawFlags flags, f32 thickness)
	{
		convert(this)->AddRect(to_imvec(min), to_imvec(max), col, rounding, flags, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_rect(Vec2 min, Vec2 max, u32 col, f32 rounding, DrawFlags flags)
	{
		convert(this)->AddRectFilled(to_imvec(min), to_imvec(max), col, rounding, flags);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_rect_multi_color(Vec2 min, Vec2 max, u32 col_ul, u32 col_ur, u32 col_br, u32 col_bl)
	{
		convert(this)->AddRectFilledMultiColor(to_imvec(min), to_imvec(max), col_ul, col_ur, col_br, col_bl);
		return *this;
	}

	DrawListHandle& DrawListHandle::quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, u32 col, f32 thickness)
	{
		convert(this)->AddQuad(to_imvec(p1), to_imvec(p2), to_imvec(p3), to_imvec(p4), col, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, u32 col)
	{
		convert(this)->AddQuadFilled(to_imvec(p1), to_imvec(p2), to_imvec(p3), to_imvec(p4), col);
		return *this;
	}

	DrawListHandle& DrawListHandle::triangle(Vec2 p1, Vec2 p2, Vec2 p3, u32 col, f32 thickness)
	{
		convert(this)->AddTriangle(to_imvec(p1), to_imvec(p2), to_imvec(p3), col, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_triangle(Vec2 p1, Vec2 p2, Vec2 p3, u32 col)
	{
		convert(this)->AddTriangleFilled(to_imvec(p1), to_imvec(p2), to_imvec(p3), col);
		return *this;
	}

	DrawListHandle& DrawListHandle::circle(Vec2 center, f32 radius, u32 col, u32 segments, f32 thickness)
	{
		convert(this)->AddCircle(to_imvec(center), radius, col, segments, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_circle(Vec2 center, f32 radius, u32 col, u32 segments)
	{
		convert(this)->AddCircleFilled(to_imvec(center), radius, col, segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::ngon(Vec2 center, f32 radius, u32 col, u32 segments, f32 thickness)
	{
		convert(this)->AddNgon(to_imvec(center), radius, col, segments, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_ngon(Vec2 center, f32 radius, u32 col, u32 segments)
	{
		convert(this)->AddNgonFilled(to_imvec(center), radius, col, segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::ellipse(Vec2 center, Vec2 radius, u32 col, f32 rot, u32 segments, f32 thickness)
	{
		convert(this)->AddEllipse(to_imvec(center), to_imvec(radius), col, rot, segments, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_ellipse(Vec2 center, Vec2 radius, u32 col, f32 rot, u32 segments)
	{
		convert(this)->AddEllipseFilled(to_imvec(center), to_imvec(radius), col, rot, segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::text(Vec2 pos, u32 col, const char* text_begin, const char* text_end)
	{
		convert(this)->AddText(to_imvec(pos), col, text_begin, text_end);
		return *this;
	}

	DrawListHandle& DrawListHandle::cubic_bezier(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, u32 col, f32 thickness, u32 segments)
	{
		convert(this)->AddBezierCubic(to_imvec(p1), to_imvec(p2), to_imvec(p3), to_imvec(p4), col, thickness, segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::quadratic_bezier(Vec2 p1, Vec2 p2, Vec2 p3, u32 col, f32 thickness, u32 segments)
	{
		convert(this)->AddBezierQuadratic(to_imvec(p1), to_imvec(p2), to_imvec(p3), col, thickness, segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::polyline(const Vec2* points, u32 count, u32 col, DrawFlags flags, f32 thickness)
	{
		convert(this)->AddPolyline(reinterpret_cast<const ImVec2*>(points), count, col, flags, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_convex_poly(const Vec2* points, u32 count, u32 col)
	{
		convert(this)->AddConvexPolyFilled(reinterpret_cast<const ImVec2*>(points), count, col);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_concave_poly(const Vec2* points, u32 count, u32 col)
	{
		convert(this)->AddConcavePolyFilled(reinterpret_cast<const ImVec2*>(points), count, col);
		return *this;
	}

	DrawListHandle& DrawListHandle::image(RHITexture* texture, Vec2 min, Vec2 max, Vec2 uv_min, Vec2 uv_max, u32 col)
	{
		convert(this)->AddImage(convert(texture), to_imvec(min), to_imvec(max), to_imvec(uv_min), to_imvec(uv_max), col);
		return *this;
	}

	DrawListHandle& DrawListHandle::image_quad(const Texture& texture, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 uv1, Vec2 uv2,
	                                           Vec2 uv3, Vec2 uv4, u32 col)
	{
		convert(this)->AddImageQuad(convert(texture), to_imvec(p1), to_imvec(p2), to_imvec(p3), to_imvec(p4), to_imvec(uv1),
		                            to_imvec(uv2), to_imvec(uv3), to_imvec(uv4), col);

		return *this;
	}

	DrawListHandle& DrawListHandle::rounded_image(const Texture& texture, Vec2 min, Vec2 max, Vec2 uv_min, Vec2 uv_max, u32 col,
	                                              f32 rounding, DrawFlags flags)
	{
		convert(this)->AddImageRounded(convert(texture), to_imvec(min), to_imvec(max), to_imvec(uv_min), to_imvec(uv_max), col,
		                               rounding, flags);

		return *this;
	}

	DrawListHandle& DrawListHandle::clear_path()
	{
		convert(this)->PathClear();
		return *this;
	}

	DrawListHandle& DrawListHandle::path_to(Vec2 pos)
	{
		convert(this)->PathLineTo(to_imvec(pos));
		return *this;
	}

	DrawListHandle& DrawListHandle::path_to_unique(Vec2 pos)
	{
		convert(this)->PathLineToMergeDuplicate(to_imvec(pos));
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_path_convex(u32 col)
	{
		convert(this)->PathFillConvex(col);
		return *this;
	}

	DrawListHandle& DrawListHandle::fill_path_concave(u32 col)
	{
		convert(this)->PathFillConcave(col);
		return *this;
	}

	DrawListHandle& DrawListHandle::stroke_path(u32 col, DrawFlags flags, f32 thickness)
	{
		convert(this)->PathStroke(col, flags, thickness);
		return *this;
	}

	DrawListHandle& DrawListHandle::arc_to(Vec2 center, f32 radius, f32 min_angle, f32 max_angle, u32 segments)
	{
		convert(this)->PathArcTo(to_imvec(center), radius, min_angle, max_angle, segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::fast_arc_to(Vec2 center, f32 radius, u32 min_12, u32 max_12)
	{
		convert(this)->PathArcToFast(to_imvec(center), radius, min_12, max_12);
		return *this;
	}

	DrawListHandle& DrawListHandle::ellipse_arc_to(Vec2 center, Vec2 radius, f32 rot, f32 min_angle, f32 max_angle, u32 segments)
	{
		convert(this)->PathEllipticalArcTo(to_imvec(center), to_imvec(radius), rot, min_angle, max_angle, segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::cubic_bezier_to(Vec2 p2, Vec2 p3, Vec2 p4, int segments)
	{
		convert(this)->PathBezierCubicCurveTo(to_imvec(p2), to_imvec(p3), to_imvec(p4), segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::quadratic_bezier_to(Vec2 p2, Vec2 p3, u32 segments)
	{
		convert(this)->PathBezierQuadraticCurveTo(to_imvec(p2), to_imvec(p3), segments);
		return *this;
	}

	DrawListHandle& DrawListHandle::path_rect(Vec2 min, Vec2 max, f32 rounding, DrawFlags flags)
	{
		convert(this)->PathRect(to_imvec(min), to_imvec(max), rounding, flags);
		return *this;
	}

	DrawListHandle& DrawListHandle::split_channels(u32 count)
	{
		convert(this)->ChannelsSplit(count);
		return *this;
	}

	DrawListHandle& DrawListHandle::merge_channels()
	{
		convert(this)->ChannelsMerge();
		return *this;
	}

	DrawListHandle& DrawListHandle::channel(u32 value)
	{
		convert(this)->ChannelsSetCurrent(value);
		return *this;
	}

}// namespace Trinex::UI
