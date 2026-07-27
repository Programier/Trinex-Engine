#include <UI/reflection.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace Trinex::UI::Refl
{
	trinex_on_pre_init()
	{
		auto refl = NativeType<ImGuiContext*>::instance();
		refl->bind("content", ImGui::GetContentRegionAvail, Property::Markup);
	}
}// namespace Trinex::UI::Refl
