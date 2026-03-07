#pragma once

struct ImGuiContext;
struct ImFont;

namespace Trinex::EditorTheme
{
	void initialize_theme(ImGuiContext* ctx);
	ImFont* small_font();
	ImFont* normal_font();
	ImFont* large_font();
}// namespace Trinex::EditorTheme
