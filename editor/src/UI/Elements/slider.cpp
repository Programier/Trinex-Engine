#include <UI/Elements/slider.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	trinex_implement_ui_element(SliderFloat)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("min", &This::min);
		reflection()->bind("max", &This::max);
		reflection()->bind("format", &This::format);
		trinex_ui_bind_property(grab_min_size);
		trinex_ui_bind_property(grab_rounding);
		trinex_ui_bind_property(grab_color);
	}

	SliderFloat& SliderFloat::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_GrabMinSize, grab_min_size);
		push_style_var(ImGuiStyleVar_GrabRounding, grab_rounding);
		push_style_color(ImGuiCol_SliderGrab, grab_color);
		push_style_color(ImGuiCol_SliderGrabActive, grab_color);
		return *this;
	}

	SliderFloat& SliderFloat::pop_style()
	{
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags SliderFloat::on_begin_update()
	{
		return item_state_flags(readback_if(ImGui::SliderFloat(label.c_str(), &value, min, max, format.c_str())));
	}

	trinex_implement_ui_element(SliderInt)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("value", &This::value);
		reflection()->bind("min", &This::min);
		reflection()->bind("max", &This::max);
		reflection()->bind("format", &This::format);
		trinex_ui_bind_property(grab_min_size);
		trinex_ui_bind_property(grab_rounding);
		trinex_ui_bind_property(grab_color);
	}

	SliderInt& SliderInt::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_GrabMinSize, grab_min_size);
		push_style_var(ImGuiStyleVar_GrabRounding, grab_rounding);
		push_style_color(ImGuiCol_SliderGrab, grab_color);
		push_style_color(ImGuiCol_SliderGrabActive, grab_color);
		return *this;
	}

	SliderInt& SliderInt::pop_style()
	{
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags SliderInt::on_begin_update()
	{
		return item_state_flags(readback_if(ImGui::SliderInt(label.c_str(), &value, min, max, format.c_str())));
	}
}// namespace Trinex::UI
