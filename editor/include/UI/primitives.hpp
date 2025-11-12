#pragma once
#include <imgui.h>

namespace Engine::UI
{
	using IconDrawFunc = void (*)(ImVec2, float, float);

	ImVec2 icon_size(float size = 0.f);

	void plus_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);
	void minus_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);
	void camera_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);
	void more_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);
	void move_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);
	void rotate_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);
	void scale_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);
	void select_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);
	void globe_icon(ImVec2 pos, float size = 0.f, float scale = 1.f);


	bool icon_button(IconDrawFunc func, const char* str_id, float size = 0.f, float scale = 1.f, ImGuiButtonFlags flags = 0);
}// namespace Engine::UI
