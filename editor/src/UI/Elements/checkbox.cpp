#include <Core/math/math.hpp>
#include <UI/Elements/checkbox.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace Trinex::UI
{
	static f32 apply_ease(f32 t, Ease mode)
	{
		t = Math::clamp(t, 0.0f, 1.0f);

		switch (mode)
		{
			case Ease::Linear: return t;
			case Ease::InQuad: return t * t;
			case Ease::OutQuad: return 1.0f - (1.0f - t) * (1.0f - t);
			case Ease::InOutQuad: return t < 0.5f ? 2.0f * t * t : 1.0f - Math::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
			case Ease::InCubic: return t * t * t;
			case Ease::OutCubic: return 1.0f - Math::pow(1.0f - t, 3.0f);
			case Ease::InOutCubic: return t < 0.5f ? 4.0f * t * t * t : 1.0f - Math::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
			case Ease::InExpo: return t == 0.0f ? 0.0f : Math::pow(2.0f, 10.0f * t - 10.0f);
			case Ease::OutExpo: return t == 1.0f ? 1.0f : 1.0f - Math::pow(2.0f, -10.0f * t);
			case Ease::InOutExpo:
				if (t == 0.0f || t == 1.0f)
					return t;
				return t < 0.5f ? Math::pow(2.0f, 20.0f * t - 10.0f) * 0.5f : (2.0f - Math::pow(2.0f, -20.0f * t + 10.0f)) * 0.5f;
			case Ease::OutBack:
			{
				const f32 c1 = 1.70158f;
				const f32 c3 = c1 + 1.0f;
				return 1.0f + c3 * Math::pow(t - 1.0f, 3.0f) + c1 * Math::pow(t - 1.0f, 2.0f);
			}
		}

		return t;
	}

	trinex_implement_ui_element(Checkbox)
	{
		trinex_ui_bind_property(label, Markup);
		trinex_ui_bind_property(value, Markup);
		trinex_ui_bind_property(check_color, Style);
		trinex_ui_bind_property(check_animation_duration, Style);
		trinex_ui_bind_property(check_animation_ease, Style);
	}

	Checkbox& Checkbox::push_scope()
	{
		Super::push_scope();
		push_style_color(ImGuiCol_CheckMark, check_color);
		push_style_color(ImGuiCol_CheckboxSelectedBg, color);
		return *this;
	}

	Checkbox& Checkbox::pop_scope()
	{
		ImGui::PopStyleColor(2);
		return *Super::pop_scope().as<This>();
	}

	Element::UpdateFlags Checkbox::on_begin_update()
	{
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

		bool hovered = false;
		bool held    = false;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

		if (pressed)
		{
			value = !value;
			ImGui::MarkItemEdited(id);
		}

		if (m_check_progress < 0.0f)
		{
			m_check_progress = value ? 1.0f : 0.0f;
		}

		const f32 target = value ? 1.0f : 0.0f;
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

		ImGui::RenderNavCursor(total_bb, id);

		const ImU32 bg_color = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
		                                          : hovered         ? ImGuiCol_FrameBgHovered
		                                          : value           ? ImGuiCol_CheckboxSelectedBg
		                                                            : ImGuiCol_FrameBg);
		ImGui::RenderFrame(check_bb.Min, check_bb.Max, bg_color, true, style.FrameRounding);

		const f32 eased = apply_ease(m_check_progress, check_animation_ease);
		if (eased > 0.0f)
		{
			const f32 pad        = Math::max(1.0f, Math::trunc(square_size / 6.0f));
			const f32 full_size  = square_size - pad * 2.0f;
			const f32 check_size = full_size * eased;
			ImVec4 color         = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
			color.w *= eased;

			ImGui::RenderCheckMark(window->DrawList,
			                       check_bb.Min +
			                               ImVec2(pad + (full_size - check_size) * 0.5f, pad + (full_size - check_size) * 0.5f),
			                       ImGui::GetColorU32(color), check_size);
		}

		const ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
		if (label_size.x > 0.0f)
		{
			ImGui::RenderText(label_pos, label.c_str(), label_end, false);
		}

		return item_state_flags(readback_if(pressed));
	}

}// namespace Trinex::UI
