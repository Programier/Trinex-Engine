#include <UI/Elements/label.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Label)
	{
		reflection()->bind("text", &This::text);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags Label::on_begin_update()
	{
		UI::label(text, value);
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
