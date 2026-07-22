#include <UI/Elements/text.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Text)
	{
		reflection()->bind("text", &This::text);
	}

	Element::UpdateFlags Text::on_begin_update()
	{
		UI::text(text);
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
