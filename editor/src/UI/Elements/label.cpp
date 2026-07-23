#include <UI/Elements/label.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Label)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags Label::on_begin_update()
	{
		UI::label(label, value);
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
