#include <UI/Elements/text.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Text)
	{
		trinex_ui_bind_property(text);
		trinex_ui_bind_property(color);
	}

	Element::UpdateFlags Text::on_begin_update()
	{
		UI::text(text);
		return UpdateFlags::Undefined;
	}
}// namespace Trinex::UI
