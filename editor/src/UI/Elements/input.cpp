#include <UI/Elements/input.hpp>
#include <UI/reflection.hpp>
#include <algorithm>
#include <cstring>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(InputFloat)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("format", &This::format);
	}

	Element::UpdateFlags InputFloat::on_begin_update()
	{
		return item_state_flags(readback_if(ImGui::InputFloat(label.c_str(), &value, 0.0f, 0.0f, format.c_str())));
	}

	trinex_implement_ui_element(InputInt)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
	}

	Element::UpdateFlags InputInt::on_begin_update()
	{
		return item_state_flags(readback_if(ImGui::InputInt(label.c_str(), &value)));
	}

	trinex_implement_ui_element(InputText)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("hint", &This::hint);
		reflection()->bind("value", &This::value);
		trinex_ui_bind_property(cursor_color, Style);
	}

	InputText& InputText::push_scope()
	{
		Super::push_scope();
		push_style_color(ImGuiCol_InputTextCursor, cursor_color);
		push_style_color(ImGuiCol_TextSelectedBg, color);
		return *this;
	}

	InputText& InputText::pop_scope()
	{
		ImGui::PopStyleColor(2);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags InputText::on_begin_update()
	{
		char buffer[1024] = {};
		const usize size  = std::min(value.size(), sizeof(buffer) - 1);
		std::memcpy(buffer, value.data(), size);

		const bool changed = hint.empty() ? ImGui::InputText(label.c_str(), buffer, sizeof(buffer))
		                                  : ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buffer, sizeof(buffer));

		if (changed)
		{
			value = buffer;
		}

		return item_state_flags(readback_if(changed));
	}
}// namespace Trinex::UI
