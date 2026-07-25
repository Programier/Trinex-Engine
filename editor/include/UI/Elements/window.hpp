#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Window : public Element
	{
		trinex_ui_element(Window, Element);

	public:
		String title;

		Vec2 padding     = {0.f, 0.f};
		Vec2 min_size    = {200.f, 200.f};
		Vec2 title_align = {0.5f, 0.5f};
		f32 rounding     = 0.f;
		f32 border_size  = 0.f;

	public:
		Window& push_style() override;
		Window& pop_style() override;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};
}// namespace Trinex::UI
