#include <Core/etl/stack.hpp>
#include <Core/math/math.hpp>
#include <UI/Elements/controls.hpp>
#include <UI/Elements/document.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace Trinex::UI
{
	static Element::UpdateFlags handle_click(Element* element, bool clicked, Name event)
	{
		if (clicked)
		{
			element->dispatch(event);
		}

		return Element::item_state_flags(Element::readback_if(clicked));
	}

	trinex_implement_ui_element(SmallButton) {}

	Element::UpdateFlags SmallButton::on_begin_update()
	{
		const bool clicked = ImGui::SmallButton(label.c_str());
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(IconButton)
	{
		trinex_ui_bind_property(icon, Markup);
	}

	Element::UpdateFlags IconButton::on_begin_update()
	{
		const String text  = icon.empty() ? label : icon + " " + label;
		const bool clicked = ImGui::Button(text.c_str(), resolve(size));
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(InvisibleButton)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(size, Markup);
		trinex_ui_bind_property(on_click, Markup);
	}

	Element::UpdateFlags InvisibleButton::on_begin_update()
	{
		const bool clicked = ImGui::InvisibleButton(label.c_str(), resolve(size));
		return handle_click(this, clicked, on_click);
	}

	RadioGroup* RadioGroup::s_current = nullptr;

	trinex_implement_ui_element(RadioGroup)
	{
		trinex_ui_bind_property(value, Markup);
	}

	RadioGroup* RadioGroup::current()
	{
		return s_current;
	}

	RadioGroup& RadioGroup::push_scope()
	{
		Super::push_scope();
		stack()->push<RadioGroup*>(s_current);
		s_current = this;

		return *this;
	}

	RadioGroup& RadioGroup::pop_scope()
	{
		s_current = *stack()->pop<RadioGroup*>();
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags RadioGroup::on_begin_update()
	{
		UpdateFlags result = UpdateFlags::Default | update_flags;
		update_flags       = UpdateFlags::Undefined;
		return result;
	}

	trinex_implement_ui_element(RadioOption)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(option, Markup);
		trinex_ui_bind_property(on_click, Markup);
		trinex_ui_bind_property(check_color, Style);
		trinex_ui_bind_property(check_animation_duration, Style);
		trinex_ui_bind_property(check_animation_ease, Style);
	}

	RadioOption& RadioOption::push_scope()
	{
		Super::push_scope();
		push_style_color(ImGuiCol_CheckMark, check_color);
		return *this;
	}

	RadioOption& RadioOption::pop_scope()
	{
		ImGui::PopStyleColor();
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags RadioOption::on_begin_update()
	{
		RadioGroup* group = RadioGroup::current();
		if (group == nullptr)
		{
			return item_state_flags();
		}

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
		{
			return UpdateFlags::Undefined;
		}

		ImGuiContext& g         = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id        = window->GetID(label.c_str());
		const char* label_end   = ImGui::FindRenderedTextEnd(label.c_str());
		const ImVec2 label_size = ImGui::CalcTextSize(label.c_str(), label_end, false);

		const f32 square_size = ImGui::GetFrameHeight();
		const ImVec2 pos      = window->DC.CursorPos;
		const ImRect check_bb(pos, pos + ImVec2(square_size, square_size));
		const ImRect total_bb(pos,
		                      pos + ImVec2(square_size + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f),
		                                   label_size.y + style.FramePadding.y * 2.0f));

		ImGui::ItemSize(total_bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(total_bb, id))
		{
			return UpdateFlags::Undefined;
		}

		bool hovered       = false;
		bool held          = false;
		const bool clicked = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

		if (clicked)
		{
			group->value = option;
			group->update_flags |= UpdateFlags::Readback;
			ImGui::MarkItemEdited(id);
			dispatch(on_click);
		}

		const bool active = group->value == option;
		if (m_check_progress < 0.0f)
		{
			m_check_progress = active ? 1.0f : 0.0f;
		}

		const f32 target = active ? 1.0f : 0.0f;
		if (check_animation_duration <= 0.0f)
		{
			m_check_progress = target;
		}
		else
		{
			const f32 step = ImGui::GetIO().DeltaTime / check_animation_duration;
			m_check_progress += (target > m_check_progress ? step : -step);
			m_check_progress = Math::clamp(m_check_progress, 0.0f, 1.0f);
		}

		ImVec2 center = check_bb.GetCenter();
		center.x      = IM_ROUND(center.x);
		center.y      = IM_ROUND(center.y);

		const f32 radius       = (square_size - 1.0f) * 0.5f;
		const i32 num_segments = window->DrawList->_CalcCircleAutoSegmentCount(radius);

		ImGui::RenderNavCursor(total_bb, id);
		window->DrawList->AddCircleFilled(center, radius,
		                                  ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
		                                                     : hovered         ? ImGuiCol_FrameBgHovered
		                                                                       : ImGuiCol_FrameBg),
		                                  num_segments);

		const f32 eased = ease(m_check_progress, check_animation_ease);

		if (eased > 0.0f)
		{
			const f32 pad          = Math::max(1.0f, Math::trunc(square_size / 6.0f));
			const f32 check_radius = (radius - pad) * eased;
			ImVec4 color           = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
			color.w *= eased;

			window->DrawList->AddCircleFilled(center, check_radius, ImGui::GetColorU32(color),
			                                  window->DrawList->_CalcCircleAutoSegmentCount(check_radius));
		}

		if (style.FrameBorderSize > 0.0f)
		{
			window->DrawList->AddCircle(center + ImVec2(1, 1), radius, ImGui::GetColorU32(ImGuiCol_BorderShadow), num_segments,
			                            style.FrameBorderSize);
			window->DrawList->AddCircle(center, radius, ImGui::GetColorU32(ImGuiCol_Border), num_segments, style.FrameBorderSize);
		}

		const ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
		if (g.LogEnabled)
		{
			ImGui::LogRenderedText(&label_pos, active ? "(x)" : "( )");
		}

		if (label_size.x > 0.0f)
		{
			ImGui::RenderText(label_pos, label.c_str(), label_end, false);
		}

		return item_state_flags(readback_if(clicked));
	}

	trinex_implement_ui_element(Selectable)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(selected, Markup);
		trinex_ui_bind_property(size, Markup);
		trinex_ui_bind_property(on_click, Markup);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(text_align, Style);
		trinex_ui_bind_property(background_color, Style);
	}

	Selectable& Selectable::push_scope()
	{
		Super::push_scope();
		push_style_var(ImGuiStyleVar_SelectableRounding, rounding);
		push_style_var(ImGuiStyleVar_SelectableTextAlign, text_align);
		push_style_color(ImGuiCol_Header, background_color);
		push_style_color(ImGuiCol_HeaderHovered, background_color);
		push_style_color(ImGuiCol_HeaderActive, background_color);
		return *this;
	}

	Selectable& Selectable::pop_scope()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags Selectable::on_begin_update()
	{
		const bool clicked = ImGui::Selectable(label.c_str(), selected, 0, resolve(size));
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(ProgressBar)
	{
		trinex_ui_bind_property(value, Markup);
		trinex_ui_bind_property(size, Markup);
		trinex_ui_bind_property(overlay, Markup);
		trinex_ui_bind_property(color, Style);
	}

	ProgressBar& ProgressBar::push_scope()
	{
		Super::push_scope();
		push_style_color(ImGuiCol_PlotHistogram, color);
		push_style_color(ImGuiCol_PlotHistogramHovered, color);
		return *this;
	}

	ProgressBar& ProgressBar::pop_scope()
	{
		ImGui::PopStyleColor(2);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags ProgressBar::on_begin_update()
	{
		ImGui::ProgressBar(value, resolve(size), overlay.empty() ? nullptr : overlay.c_str());
		return item_state_flags();
	}

	trinex_implement_ui_element(ColorEdit)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(value, Markup);
	}

	Element::UpdateFlags ColorEdit::on_begin_update()
	{
		const bool edited = ImGui::ColorEdit4(label.c_str(), &value.x);
		return item_state_flags(readback_if(edited));
	}
}// namespace Trinex::UI
