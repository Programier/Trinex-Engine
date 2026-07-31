#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Visual : public Element
	{
		trinex_ui_element(Visual, Element);

	public:
		ImVec2 pivot     = {0.5, 0.5};
		ImVec2 translate = {0.f, 0.f};
		ImVec2 scale     = {1.f, 1.f};
		f32 rotate       = 0.f;

		Visual& push_style() override;
		Visual& pop_style() override;
	};

	class DrawNode : public Visual
	{
		trinex_ui_element(DrawNode, Visual);

	public:
		Vec2 offset = {0.0f, 0.0f};
	};

	class Line : public DrawNode
	{
		trinex_ui_element(Line, DrawNode);

	public:
		Vec2 p1       = {0.0f, 0.0f};
		Vec2 p2       = {0.0f, 0.0f};
		Vec4 color    = {1.0f, 1.0f, 1.0f, 1.0f};
		f32 thickness = 1.0f;

		UpdateFlags on_begin_update() override;
	};

	class Rect : public DrawNode
	{
		trinex_ui_element(Rect, DrawNode);

	public:
		Vec2 min      = {0.0f, 0.0f};
		Vec2 max      = {0.0f, 0.0f};
		Vec4 color    = {1.0f, 1.0f, 1.0f, 1.0f};
		f32 rounding  = 0.0f;
		f32 thickness = 1.0f;
		i32 flags     = 0;
		bool filled   = false;

		UpdateFlags on_begin_update() override;
	};

	class Circle : public DrawNode
	{
		trinex_ui_element(Circle, DrawNode);

	public:
		Vec2 center   = {0.0f, 0.0f};
		f32 radius    = 0.0f;
		Vec4 color    = {1.0f, 1.0f, 1.0f, 1.0f};
		i32 segments  = 0;
		f32 thickness = 1.0f;
		bool filled   = false;

		UpdateFlags on_begin_update() override;
	};

	class Triangle : public DrawNode
	{
		trinex_ui_element(Triangle, DrawNode);

	public:
		Vec2 p1       = {0.0f, 0.0f};
		Vec2 p2       = {0.0f, 0.0f};
		Vec2 p3       = {0.0f, 0.0f};
		Vec4 color    = {1.0f, 1.0f, 1.0f, 1.0f};
		f32 thickness = 1.0f;
		bool filled   = false;

		UpdateFlags on_begin_update() override;
	};

	class Polyline : public DrawNode
	{
		trinex_ui_element(Polyline, DrawNode);

	public:
		Vector<ImVec2> points;
		Vec4 color    = {1.0f, 1.0f, 1.0f, 1.0f};
		i32 flags     = 0;
		f32 thickness = 1.0f;
		bool closed   = false;

		UpdateFlags on_begin_update() override;
	};

	class Bezier : public DrawNode
	{
		trinex_ui_element(Bezier, DrawNode);

	public:
		Vec2 p1       = {0.0f, 0.0f};
		Vec2 p2       = {0.0f, 0.0f};
		Vec2 p3       = {0.0f, 0.0f};
		Vec2 p4       = {0.0f, 0.0f};
		Vec4 color    = {1.0f, 1.0f, 1.0f, 1.0f};
		f32 thickness = 1.0f;
		i32 segments  = 0;

		UpdateFlags on_begin_update() override;
	};

	class DrawText : public DrawNode
	{
		trinex_ui_element(DrawText, DrawNode);

	public:
		Vec2 pos      = {0.0f, 0.0f};
		String text;
		Vec4 color    = {1.0f, 1.0f, 1.0f, 1.0f};
		f32 font_size = 0.0f;

		UpdateFlags on_begin_update() override;
	};

	class Image : public DrawNode
	{
		trinex_ui_element(Image, DrawNode);

	public:
		Texture texture;
		Vec2 min    = {0.0f, 0.0f};
		Vec2 max    = {0.0f, 0.0f};
		Vec2 uv_min = {0.0f, 0.0f};
		Vec2 uv_max = {1.0f, 1.0f};
		Vec4 color  = {1.0f, 1.0f, 1.0f, 1.0f};

		UpdateFlags on_begin_update() override;
	};

	class ClipRect : public DrawNode
	{
		trinex_ui_element(ClipRect, DrawNode);

	public:
		Vec2 min       = {0.0f, 0.0f};
		Vec2 max       = {0.0f, 0.0f};
		bool intersect = true;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};
}// namespace Trinex::UI
