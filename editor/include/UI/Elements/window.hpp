#pragma once
#include <UI/Styles/tab.hpp>
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Window : public Element
	{
		trinex_ui_element(Window, Element);

	public:
		String title;

		TabStyle tab;

		Size padding           = Size(0.0f, 0.0f);
		Size min_size          = Size(200.0f, 200.0f);
		Vec2 title_align       = {0.5f, 0.5f};
		Unit rounding          = Unit(0.0f);
		Unit border_size       = Unit(0.0f);
		Vec4 background_color  = {0.06f, 0.06f, 0.10f, 0.94f};
		Vec4 title_color       = {0.09f, 0.09f, 0.09f, 1.00f};
		Vec4 resize_grip_color = {0.20f, 0.25f, 0.30f, 0.94f};

		ImGuiWindowFlags window_flags = 0;

	public:
		Window& push_scope() override;
		Window& pop_scope() override;

		UpdateFlags on_begin_update() override;
		Element& on_end_update(UpdateFlags flags) override;
	};
}// namespace Trinex::UI
