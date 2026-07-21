#include <UI/Elements/text.hpp>
#include <UI/api.hpp>
#include <UI/element_registry.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Text)
	{
		reflection()->property<&This::text>("text");
	}

	bool Text::on_begin_render()
	{
		UI::text(text);
		return false;
	}
}// namespace Trinex::UI
