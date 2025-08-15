#include <Core/imgui_config.hpp>
#include <Graphics/texture.hpp>

ImGuiContext*& get_current_imgui_context()
{
	static thread_local ImGuiContext* current_context = nullptr;
	return current_context;
}
