#pragma once
#include <imgui.h>


namespace Engine::EditorTheme
{
	extern ImVec4 table_row_color1;
	extern ImVec4 table_row_color2;

	void initialize_theme(ImGuiContext* ctx);
	float editor_scale_factor();
	float editor_font_size();
}// namespace Engine::EditorTheme
