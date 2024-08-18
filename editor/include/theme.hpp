#pragma once

struct ImGuiContext;

namespace Engine
{
	void initialize_theme(ImGuiContext* ctx);
	float editor_scale_factor();
	float editor_font_size();
}// namespace Engine
