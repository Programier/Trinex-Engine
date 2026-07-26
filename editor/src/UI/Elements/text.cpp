#include <UI/Elements/text.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Text)
	{
		trinex_ui_bind_property(text, Markup);
		trinex_ui_bind_property(color, Style);
	}

	Element::UpdateFlags Text::on_begin_update()
	{
		if (color.w > 0.0f)
		{
			ImGui::TextColored(ImVec4(color.x, color.y, color.z, color.w), "%s", text.c_str());
		}
		else
		{
			ImGui::TextUnformatted(text.c_str());
		}
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
