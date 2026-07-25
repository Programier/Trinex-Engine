#include <UI/Elements/label.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(Label)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags Label::on_begin_update()
	{
		ImGui::LabelText(label.c_str(), "%s", value.c_str());
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
