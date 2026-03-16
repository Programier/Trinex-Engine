#pragma once

struct ImGuiContext;
struct ImFont;

namespace Trinex::UI
{
	enum class FontSize
	{
		Small  = 0,
		Normal = 1,
		Large  = 2,
	};

	void initialize_theme(ImGuiContext* ctx);

	ImFont* text_font(FontSize size = FontSize::Normal);
	ImFont* icons_font(FontSize size = FontSize::Normal);
}// namespace Trinex::UI
