#include <UI/Elements/input.hpp>
#include <UI/api.hpp>
#include <UI/reflection.hpp>
#include <algorithm>
#include <cstring>

namespace Trinex::UI
{
	trinex_implement_ui_element(InputFloat)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("format", &This::format);
	}

	bool InputFloat::on_begin_render()
	{
		UI::input(label, &value, format.c_str());
		return false;
	}

	trinex_implement_ui_element(InputInt)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	bool InputInt::on_begin_render()
	{
		UI::input(label, &value);
		return false;
	}

	trinex_implement_ui_element(InputText)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("hint", &This::hint);
		reflection()->bind("value", &This::value);
	}

	bool InputText::on_begin_render()
	{
		char buffer[1024] = {};
		const usize size  = std::min(value.size(), sizeof(buffer) - 1);
		std::memcpy(buffer, value.data(), size);

		const bool changed = hint.empty() ? UI::input(label, buffer, sizeof(buffer)) : UI::input(label, hint, buffer, sizeof(buffer));

		if (changed)
		{
			value = buffer;
		}

		return false;
	}
}// namespace Trinex::UI
