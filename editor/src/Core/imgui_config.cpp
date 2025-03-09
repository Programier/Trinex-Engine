#include <Core/imgui_config.hpp>
#include <Graphics/texture_2D.hpp>

ImGuiContext*& get_current_imgui_context()
{
	static thread_local ImGuiContext* current_context = nullptr;
	return current_context;
}
