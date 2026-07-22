#include <UI/Elements/text.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Text)
	{
		reflection()->bind("text", &This::text);
	}

	bool Text::on_begin_render()
	{
		UI::text(text);
		return false;
	}
}// namespace Trinex::UI
