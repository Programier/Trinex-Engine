#include "api_internal.hpp"
#include <Core/math/math.hpp>
#include <cstring>

namespace Trinex::UI
{
	/////////////////////// IMAGES AND CONTROLS ///////////////////////

	void image(const Texture& texture, Size size, const ImageOptions& options)
	{
		const Vec2 resolved_size = resolve(size);
		if (texture == nullptr)
		{
			ImGui::Dummy(to_imvec(resolved_size));
			return;
		}

		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding  = Math::max(0.0f, options.padding);
		const ImVec2 image_size(Math::max(0.0f, resolved_size.x), Math::max(0.0f, resolved_size.y));
		const ImVec2 frame_size(image_size.x + padding * 2.0f, image_size.y + padding * 2.0f);
		const ImVec2 pos = ImGui::GetCursorScreenPos();

		ImGui::Dummy(frame_size);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 max = add(pos, frame_size);
		const ImVec2 image_min(pos.x + padding, pos.y + padding);
		const ImVec2 image_max(image_min.x + image_size.x, image_min.y + image_size.y);
		const float image_rounding = Math::max(0.0f, rounding - padding * 0.5f);

		Vec4 frame_bg     = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		Vec4 frame_border = has_color(options.border_color) ? options.border_color : active_context()->style.colors.border;

		if ((options.background || options.border) && has_shadow_override())
		{
			draw_shadow_rect(draw, pos, max, rounding, current_shadow());
		}

		if (options.background)
		{
			draw->AddRectFilled(pos, max, col_u32(frame_bg), rounding);
		}

		draw->AddImageRounded(ImTextureID(texture.texture, texture.sampler), image_min, image_max, to_imvec(options.uv0),
		                      to_imvec(options.uv1), col_u32(options.tint), image_rounding);

		if (options.border)
		{
			draw->AddRect(pos, max, col_u32(frame_border), rounding, 0, active_context()->style.border_size);
		}
	}

	bool image_button(StringView id_text, const Texture& texture, Size size, const ImageOptions& options)
	{
		const Vec2 resolved_size = resolve(size);
		if (texture == nullptr)
		{
			ImGui::Dummy(to_imvec(resolved_size));
			return false;
		}

		cleanup_states();

		if (!id_text.empty())
		{
			imgui_push_id(id_text);
		}
		else
		{
			ImGui::PushID(texture.texture);
		}
		ImGui::PushID(texture.sampler);

		const ImGuiID id     = ImGui::GetID("image_button");
		AnimState& anim      = state_for(id);
		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding  = Math::max(0.0f, options.padding);
		const ImVec2 image_size(Math::max(0.0f, resolved_size.x), Math::max(0.0f, resolved_size.y));
		const ImVec2 frame_size(image_size.x + padding * 2.0f, image_size.y + padding * 2.0f);
		const ImVec2 pos = ImGui::GetCursorScreenPos();

		ImGui::InvisibleButton("##image_button", frame_size);

		const bool hovered = ImGui::IsItemHovered();
		const bool active  = ImGui::IsItemActive();
		const bool clicked = ImGui::IsItemClicked();

		anim.hover  = approach(anim.hover, hovered ? 1.0f : 0.0f);
		anim.active = approach(anim.active, active ? 1.0f : 0.0f * 1.5f);

		const Vec4 accent = has_color(options.accent) ? options.accent : active_context()->style.colors.accent;
		Vec4 frame_bg     = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		Vec4 frame_border = has_color(options.border_color) ? options.border_color : active_context()->style.colors.border;
		frame_bg          = Math::lerp(frame_bg, active_context()->style.colors.background_hovered, anim.hover * 0.55f);
		frame_bg          = Math::lerp(frame_bg, active_context()->style.colors.background_active, anim.active * 0.65f);
		frame_border      = Math::lerp(frame_border, accent, anim.hover * 0.7f + anim.active * 0.3f);

		const bool hover_scaled = anim.hover > 0.0f && (active_context()->style.hover_padding.x != 0.0f ||
		                                                active_context()->style.hover_padding.y != 0.0f);
		if (hover_scaled)
		{
			push_render_scale(make_hover_render_scale(frame_size, anim.hover), RenderScaleFlags::StartFromLastItemBounds);
		}
		const bool press_scaled = anim.active > 0.0f;
		if (press_scaled)
		{
			push_render_scale(make_press_render_scale(frame_size, anim.active), RenderScaleFlags::StartFromLastItemBounds);
		}

		ImDrawList* draw                 = ImGui::GetWindowDrawList();
		const InteractiveRect frame_rect = make_interactive_rect(pos, frame_size);
		const InteractiveRect image_rect = make_interactive_rect(ImVec2(pos.x + padding, pos.y + padding), image_size);
		const float image_rounding       = Math::max(0.0f, (rounding - padding * 0.5f) * image_rect.rounding_scale);

		if (has_shadow_override())
		{
			draw_shadow_rect(draw, frame_rect.min, frame_rect.max, rounding * frame_rect.rounding_scale, current_shadow());
		}

		if (options.background)
		{
			draw->AddRectFilled(frame_rect.min, frame_rect.max, col_u32(frame_bg), rounding * frame_rect.rounding_scale);
		}

		draw->AddImageRounded(ImTextureID(texture.texture, texture.sampler), image_rect.min, image_rect.max,
		                      to_imvec(options.uv0), to_imvec(options.uv1), col_u32(options.tint), image_rounding);

		if (options.border)
		{
			draw->AddRect(frame_rect.min, frame_rect.max, col_u32(frame_border), rounding * frame_rect.rounding_scale, 0,
			              active_context()->style.border_size);
		}

		if (press_scaled)
		{
			pop_render_scale();
		}
		if (hover_scaled)
		{
			pop_render_scale();
		}

		ImGui::PopID();
		ImGui::PopID();
		return clicked;
	}

	bool button(StringView label, const ButtonOptions& options)
	{
		cleanup_states();
		imgui_push_id(label);
		const ImGuiID id              = ImGui::GetID("button");
		AnimState& anim               = state_for(id);
		const StringView text_label   = visible_label(label);
		const bool has_text           = !text_label.empty();
		const bool has_icon           = !options.icon.empty();
		const float icon_gap          = has_icon && has_text ? active_context()->style.spacing * 0.5f : 0.0f;
		const float current_font_size = ImGui::GetFontSize();
		ImFont* icon_font             = nullptr;
		ImVec2 icon_size(0.0f, 0.0f);
		if (has_icon)
		{
			const FontSize font_size = closest_font_size(current_font_size);
			icon_font                = resolve_font(active_context(), FontFamily::Icons, font_size);
			if (icon_font != nullptr)
			{
				icon_size = icon_font->CalcTextSizeA(current_font_size, std::numeric_limits<float>::max(), 0.0f,
				                                     options.icon.data(), options.icon.data() + options.icon.size());
			}
		}

		const ImVec2 text_size = has_text ? ImGui::CalcTextSize(text_label.data(), text_label.data() + text_label.size(), true)
		                                  : ImVec2(0.0f, 0.0f);
		const ImVec2 content_size(icon_size.x + icon_gap + text_size.x, Math::max(icon_size.y, text_size.y));
		ImVec2 size = to_imvec(resolve(options.size));
		if (size.x <= 0.0f)
		{
			size.x = Math::max(72.0f, content_size.x + active_context()->style.padding * 2.0f);
		}
		if (size.y <= 0.0f)
		{
			size.y = active_context()->style.frame_height;
		}
		const ImVec2 pos = ImGui::GetCursorScreenPos();

		if (options.disabled)
		{
			ImGui::BeginDisabled();
		}
		ImGui::InvisibleButton("##button", size);
		if (options.disabled)
		{
			ImGui::EndDisabled();
		}

		const bool hovered = !options.disabled && ImGui::IsItemHovered();
		const bool active  = !options.disabled && ImGui::IsItemActive();
		const bool clicked = !options.disabled && ImGui::IsItemClicked();
		anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f);
		anim.active        = approach(anim.active, active ? 1.0f : 0.0f * 1.5f);

		const Vec4 accent       = has_color(options.accent) ? options.accent : active_context()->style.colors.accent;
		Vec4 bg                 = options.ghost ? Vec4(0, 0, 0, 0) : active_context()->style.colors.background_active;
		bg                      = Math::lerp(bg, active_context()->style.colors.background_hovered, anim.hover);
		bg                      = Math::lerp(bg, accent, anim.active * 0.65f);
		const bool hover_scaled = anim.hover > 0.0f && (active_context()->style.hover_padding.x != 0.0f ||
		                                                active_context()->style.hover_padding.y != 0.0f);
		if (hover_scaled)
		{
			push_render_scale(make_hover_render_scale(size, anim.hover), RenderScaleFlags::StartFromLastItemBounds);
		}
		const bool press_scaled = anim.active > 0.0f;
		if (press_scaled)
		{
			push_render_scale(make_press_render_scale(size, anim.active), RenderScaleFlags::StartFromLastItemBounds);
		}
		ImDrawList* draw           = ImGui::GetWindowDrawList();
		const InteractiveRect rect = make_interactive_rect(pos, size);
		if (has_shadow_override())
		{
			ShadowOptions shadow = current_shadow();
			if (options.disabled)
			{
				shadow.color.w *= 0.55f;
			}
			draw_shadow_rect(draw, rect.min, rect.max, active_context()->style.rounding * rect.rounding_scale, shadow);
		}
		if (!options.ghost || anim.hover > 0.01f || anim.active > 0.01f)
		{
			draw->AddRectFilled(rect.min, rect.max, col_u32(bg, options.disabled ? 0.45f : 1.0f),
			                    active_context()->style.rounding * rect.rounding_scale);
		}
		draw->AddRect(
		        rect.min, rect.max,
		        col_u32(Math::lerp(active_context()->style.colors.border, accent, anim.hover), options.disabled ? 0.45f : 1.0f),
		        active_context()->style.rounding * rect.rounding_scale, 0, active_context()->style.border_size);

		const Vec4 tc = options.disabled ? active_context()->style.colors.text_disabled
		                                 : Math::lerp(active_context()->style.colors.text, Vec4(1, 1, 1, 1), anim.hover * 0.35f);
		float x       = rect.center.x - content_size.x * 0.5f;
		if (has_icon && icon_font != nullptr)
		{
			draw->AddText(icon_font, current_font_size, ImVec2(x, rect.center.y - icon_size.y * 0.5f), col_u32(tc),
			              options.icon.data(), options.icon.data() + options.icon.size());
			x += icon_size.x + icon_gap;
		}
		if (has_text)
		{
			draw->AddText(ImVec2(x, rect.center.y - text_size.y * 0.5f), col_u32(tc), text_label.data(),
			              text_label.data() + text_label.size());
		}
		if (press_scaled)
		{
			pop_render_scale();
		}
		if (hover_scaled)
		{
			pop_render_scale();
		}
		ImGui::PopID();
		return clicked;
	}

	bool invisible_button(StringView label, const ButtonOptions& options)
	{
		{
			String label_storage(label);
			return ImGui::InvisibleButton(label_storage.c_str(), to_imvec(resolve(options.size)), options.flags);
		}
	}

	bool icon_button(StringView icon, StringView label, const ButtonOptions& options)
	{
		ButtonOptions copy = options;
		copy.icon          = icon;
		return button(label, copy);
	}

	bool small_button(StringView label)
	{
		ButtonOptions options;
		options.size = Size(0.0f, 24.0f);
		return button(label, options);
	}

	bool ghost_button(StringView label, Size size)
	{
		ButtonOptions options;
		options.size  = size;
		options.ghost = true;
		return button(label, options);
	}

	bool danger_button(StringView label, Size size)
	{
		ButtonOptions options;
		options.size   = size;
		options.accent = active_context()->style.colors.error;
		return button(label, options);
	}

	bool checkbox(StringView label, bool* value)
	{
		cleanup_states();
		imgui_push_id(label);
		const ImGuiID id = ImGui::GetID("checkbox");
		AnimState& anim  = state_for(id);
		const float box  = 20.0f;
		const ImVec2 ts  = imgui_calc_text_size(visible_label(label));
		const ImVec2 size(box + active_context()->style.spacing + ts.x, Math::max(box, active_context()->style.frame_height));
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##checkbox", size);
		const bool clicked = ImGui::IsItemClicked();
		if (clicked && value != nullptr)
		{
			*value = !*value;
		}
		anim.hover  = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.active = approach(anim.active, ImGui::IsItemActive() ? 1.0f : 0.0f);
		anim.value  = approach(anim.value, value != nullptr && *value ? 1.0f : 0.0f);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 box_pos(pos.x, pos.y + (size.y - box) * 0.5f);
		const Vec4 fill =
		        Math::lerp(active_context()->style.colors.background, active_context()->style.colors.accent, anim.value);
		draw->AddRectFilled(
		        box_pos, add(box_pos, ImVec2(box, box)),
		        col_u32(Math::lerp(fill, active_context()->style.colors.background_hovered, anim.hover * (1.0f - anim.value))),
		        active_context()->style.rounding * 0.55f);
		draw->AddRect(box_pos, add(box_pos, ImVec2(box, box)),
		              col_u32(Math::lerp(active_context()->style.colors.border, active_context()->style.colors.accent_hovered,
		                                 anim.hover + anim.value)),
		              active_context()->style.rounding * 0.55f, 0, active_context()->style.border_size);
		if (anim.value > 0.02f)
		{
			const float s  = apply_ease(anim.value, Ease::OutBack);
			const ImVec2 c = add(box_pos, ImVec2(box * 0.5f, box * 0.5f));
			draw->AddLine(ImVec2(c.x - 5.0f * s, c.y), ImVec2(c.x - 1.0f * s, c.y + 4.0f * s),
			              col_u32(active_context()->style.colors.text, anim.value), 2.0f);
			draw->AddLine(ImVec2(c.x - 1.0f * s, c.y + 4.0f * s), ImVec2(c.x + 6.0f * s, c.y - 5.0f * s),
			              col_u32(active_context()->style.colors.text, anim.value), 2.0f);
		}
		draw_list_add_text(draw, ImVec2(pos.x + box + active_context()->style.spacing, pos.y + (size.y - ts.y) * 0.5f),
		                   col_u32(active_context()->style.colors.text), visible_label(label));
		ImGui::PopID();
		return clicked;
	}

	bool toggle(StringView label, bool* value)
	{
		cleanup_states();
		imgui_push_id(label);
		const ImGuiID id = ImGui::GetID("toggle");
		AnimState& anim  = state_for(id);
		const ImVec2 track(46.0f, 24.0f);
		const ImVec2 ts = imgui_calc_text_size(visible_label(label));
		const ImVec2 size(track.x + active_context()->style.spacing + ts.x,
		                  Math::max(track.y, active_context()->style.frame_height));
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##toggle", size);
		const bool clicked = ImGui::IsItemClicked();
		if (clicked && value != nullptr)
		{
			*value = !*value;
		}
		anim.hover = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.value = approach(anim.value, value != nullptr && *value ? 1.0f : 0.0f);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 p(pos.x, pos.y + (size.y - track.y) * 0.5f);
		const float toggle_t = apply_ease(anim.value, Ease::InOutQuad);
		const Vec4 track_col = Math::lerp(Math::lerp(active_context()->style.colors.background_active,
		                                             active_context()->style.colors.background_hovered, anim.hover),
		                                  active_context()->style.colors.accent, toggle_t);
		draw->AddRectFilled(p, add(p, track), col_u32(track_col), track.y * 0.5f);
		const float knob_r = 9.0f + anim.hover;
		const float knob_x = Math::lerp(p.x + track.y * 0.5f, p.x + track.x - track.y * 0.5f, toggle_t);
		draw->AddCircleFilled(ImVec2(knob_x, p.y + track.y * 0.5f), knob_r, col_u32(active_context()->style.colors.text));
		draw_list_add_text(draw, ImVec2(pos.x + track.x + active_context()->style.spacing, pos.y + (size.y - ts.y) * 0.5f),
		                   col_u32(active_context()->style.colors.text), visible_label(label));
		ImGui::PopID();
		return clicked;
	}

	bool slider(StringView label, float* value, float min, float max, const char* format)
	{
		cleanup_states();
		imgui_push_id(label);
		const ImGuiID id        = ImGui::GetID("slider");
		AnimState& anim         = state_for(id);
		const float width       = ImGui::CalcItemWidth();
		const ImVec2 label_size = imgui_calc_text_size(visible_label(label));
		const ImVec2 size(width, active_context()->style.frame_height);
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##slider", size);
		bool changed = false;
		if (ImGui::IsItemActive() && value != nullptr && max > min)
		{
			const float mouse_t = Math::clamp((ImGui::GetIO().MousePos.x - pos.x) / size.x, 0.f, 1.f);
			const float next    = min + (max - min) * mouse_t;
			if (*value != next)
			{
				*value  = next;
				changed = true;
			}
		}
		const float target = value != nullptr && max > min ? Math::clamp((*value - min) / (max - min), 0.f, 1.f) : 0.0f;
		anim.value         = approach(anim.value, target);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.active        = approach(anim.active, ImGui::IsItemActive() ? 1.0f : 0.0f);

		ImDrawList* draw  = ImGui::GetWindowDrawList();
		const float bar_h = 7.0f;
		const ImVec2 bar_min(pos.x, pos.y + (size.y - bar_h) * 0.5f);
		const ImVec2 bar_max(pos.x + size.x, bar_min.y + bar_h);
		draw->AddRectFilled(bar_min, bar_max, col_u32(active_context()->style.colors.background_active), bar_h * 0.5f);
		draw->AddRectFilled(bar_min, ImVec2(Math::lerp(bar_min.x, bar_max.x, anim.value), bar_max.y),
		                    col_u32(Math::lerp(active_context()->style.colors.accent,
		                                       active_context()->style.colors.accent_hovered, anim.hover)),
		                    bar_h * 0.5f);
		draw->AddCircleFilled(ImVec2(Math::lerp(bar_min.x, bar_max.x, anim.value), bar_min.y + bar_h * 0.5f),
		                      6.0f + anim.active * 2.0f, col_u32(active_context()->style.colors.text));
		if (label_size.x > 0.0f && !visible_label(label).empty())
		{
			ImGui::SameLine();
			imgui_text_unformatted(visible_label(label));
		}
		if (value != nullptr && ImGui::IsItemHovered())
		{
			char buf[64];
			std::snprintf(buf, sizeof(buf), format, *value);
			ImGui::SetTooltip("%s", buf);
		}
		ImGui::PopID();
		return changed;
	}

	bool slider(StringView label, int* value, int min, int max, const char* format)
	{
		float v = value != nullptr ? static_cast<float>(*value) : 0.0f;
		(void) format;
		const bool changed = slider(label, &v, static_cast<float>(min), static_cast<float>(max), "%.0f");
		if (changed && value != nullptr)
		{
			*value = static_cast<int>(Math::round(v));
		}
		return changed;
	}

	bool drag(StringView label, float* value, float speed, float min, float max, const char* format)
	{
		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::DragFloat(label_storage.c_str(), value, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool drag(StringView label, int* value, float speed, int min, int max, const char* format)
	{
		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::DragInt(label_storage.c_str(), value, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool drag(StringView label, Vec2* value, float speed, float min, float max, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::DragFloat2(label_storage.c_str(), &value->x, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool drag(StringView label, Vec3* value, float speed, float min, float max, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::DragFloat3(label_storage.c_str(), &value->x, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool drag(StringView label, Vec4* value, float speed, float min, float max, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::DragFloat4(label_storage.c_str(), &value->x, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, double* value, const char* format)
	{
		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::InputDouble(label_storage.c_str(), value, 0.0, 0.0, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, float* value, const char* format)
	{
		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::InputFloat(label_storage.c_str(), value, 0.0f, 0.0f, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, int* value)
	{
		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::InputInt(label_storage.c_str(), value);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, Vec2* value, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::InputFloat2(label_storage.c_str(), &value->x, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, Vec3* value, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::InputFloat3(label_storage.c_str(), &value->x, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, Vec4* value, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::InputFloat4(label_storage.c_str(), &value->x, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, char* buffer, size_t buffer_size, InputTextFlags flags)
	{
		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::InputText(label_storage.c_str(), buffer, buffer_size, to_imgui_input_text_flags(flags));
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, StringView hint, char* buffer, size_t buffer_size, InputTextFlags flags)
	{
		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		String hint_storage(hint);
		const bool changed = ImGui::InputTextWithHint(label_storage.c_str(), hint_storage.c_str(), buffer, buffer_size,
		                                              to_imgui_input_text_flags(flags));
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(StringView label, char* buffer, size_t buffer_size, Size size, InputTextFlags flags)
	{
		cleanup_states();
		imgui_push_id(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		String label_storage(label);
		const bool changed = ImGui::InputTextMultiline(label_storage.c_str(), buffer, buffer_size, to_imvec(resolve(size)),
		                                               to_imgui_input_text_flags(flags));
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool search_input(StringView label, char* buffer, size_t buffer_size)
	{
		return input(label, buffer, buffer_size);
	}

	bool begin_combo(StringView label, StringView preview_value, ComboFlags flags)
	{
		auto& combo = active_context()->active_combo;

		cleanup_states();
		imgui_push_id(label);
		const ImGuiID id = ImGui::GetID("combo_open");
		bool& open       = active_context()->open[id];
		AnimState& anim  = state_for(id);

		const ImVec2 label_size = imgui_calc_text_size(visible_label(label));
		const float width       = ImGui::CalcItemWidth();
		const ImVec2 frame_size(width, active_context()->style.frame_height);
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##combo_field", frame_size);
		const bool hovered    = ImGui::IsItemHovered();
		const bool active     = ImGui::IsItemActive();
		const bool popup_open = ImGui::IsPopupOpen("##combo_popup", ImGuiPopupFlags_None);

		if (ImGui::IsItemClicked())
		{
			ImGuiContext* imgui_context = ImGui::GetCurrentContext();
			if (popup_open)
			{
				if (imgui_context != nullptr && imgui_context->BeginPopupStack.Size > 0)
				{
					ImGui::ClosePopupToLevel(imgui_context->BeginPopupStack.Size - 1, true);
				}
				combo = 0;
			}
			else
			{
				if (combo != 0 && combo != id)
				{
					if (imgui_context != nullptr && imgui_context->BeginPopupStack.Size > 0)
					{
						ImGui::ClosePopupToLevel(imgui_context->BeginPopupStack.Size - 1, true);
					}
					active_context()->open[combo] = false;
				}
				ImGui::OpenPopup("##combo_popup");
				combo                                    = id;
				active_context()->active_combo_field_min = pos;
				active_context()->active_combo_field_max = add(pos, frame_size);
			}
		}
		else if (combo != 0 && combo != id)
		{
			open = false;
		}

		open = ImGui::IsPopupOpen("##combo_popup", ImGuiPopupFlags_None);
		if (!open && combo == id)
		{
			combo = 0;
		}

		anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f);
		anim.active        = approach(anim.active, active ? 1.0f : 0.0f);
		anim.open          = approach(anim.open, open ? 1.0f : 0.0f);
		const float open_t = apply_ease(anim.open, Ease::InOutQuad);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const Vec4 bg    = Math::lerp(Math::lerp(active_context()->style.colors.background,
		                                         active_context()->style.colors.background_hovered, anim.hover),
		                              active_context()->style.colors.background_active, anim.active);
		draw->AddRectFilled(pos, add(pos, frame_size), col_u32(bg), active_context()->style.rounding);
		draw->AddRect(pos, add(pos, frame_size),
		              col_u32(Math::lerp(active_context()->style.colors.border, active_context()->style.colors.accent, open_t)),
		              active_context()->style.rounding, 0, active_context()->style.border_size);
		const ImVec2 preview_size = imgui_calc_text_size(preview_value);
		draw_list_add_text(draw, ImVec2(pos.x + active_context()->style.padding, pos.y + (frame_size.y - preview_size.y) * 0.5f),
		                   col_u32(active_context()->style.colors.text), preview_value);
		draw_chevron(draw, ImVec2(pos.x + frame_size.x - 16.0f, pos.y + frame_size.y * 0.5f), 9.0f, open_t,
		             col_u32(active_context()->style.colors.text_muted));

		if (label_size.x > 0.0f && !visible_label(label).empty())
		{
			ImGui::SameLine();
			imgui_text_unformatted(visible_label(label));
		}

		UI::push_render_scale(Vec2(1.f, anim.open), {0.5, 0.f});

		if (open)
		{
			ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + frame_size.y), ImGuiCond_Always);
		}
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		const bool visible = ImGui::BeginPopup("##combo_popup");
		if (visible)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, active_context()->style.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, to_imvec(active_context()->style.colors.panel));
			ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(active_context()->style.colors.border));
			ImGui::BeginChild("##combo_panel", ImVec2(width, 160.0f), true);

			if (active_context()->active_combo == id)
			{
				active_context()->active_combo_field_min = pos;
				active_context()->active_combo_field_max = add(pos, frame_size);
				active_context()->active_combo_popup_min = ImGui::GetWindowPos();
				active_context()->active_combo_popup_max = add(active_context()->active_combo_popup_min, ImGui::GetWindowSize());
			}
			return true;
		}
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
		ImGui::PopID();
		return false;
	}

	void end_combo()
	{
		ImGui::EndChild();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(2);
		ImGui::EndPopup();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
		UI::pop_render_scale();
		ImGui::PopID();
	}

	bool combo(StringView label, int* current_item, const char* const items[], int item_count)
	{
		if (current_item == nullptr || items == nullptr || item_count <= 0)
		{
			return false;
		}
		const int index = Math::max(0, Math::min(*current_item, item_count - 1));
		bool changed    = false;
		if (begin_combo(label, items[index]))
		{
			for (int i = 0; i < item_count; ++i)
			{
				if (selectable(items[i], *current_item == i))
				{
					*current_item = i;
					changed       = true;

					auto& combo = active_context()->active_combo;

					if (combo != 0)
					{
						active_context()->open[combo] = false;
						combo                         = 0;
						ImGui::CloseCurrentPopup();
					}
				}
			}
			end_combo();
		}
		return changed;
	}

	bool selectable(StringView label, bool selected, SelectableFlags flags, Size size)
	{
		TreeNodeOptions options;
		options.selected = selected;
		options.leaf     = true;
		(void) flags;
		const bool clicked = tree_leaf(label, options);
		auto& combo        = active_context()->active_combo;

		if (clicked && combo != 0)
		{
			active_context()->open[combo] = false;
			combo                         = 0;
			ImGui::CloseCurrentPopup();
		}
		return clicked;
	}

	bool radio_button(StringView label, bool active)
	{
		cleanup_states();
		imgui_push_id(label);
		const ImGuiID id       = ImGui::GetID("radio");
		AnimState& anim        = state_for(id);
		const float diameter   = 20.0f;
		const ImVec2 text_size = imgui_calc_text_size(visible_label(label));
		const ImVec2 size(diameter + active_context()->style.spacing + text_size.x,
		                  Math::max(active_context()->style.frame_height, diameter));
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##radio", size);
		const bool clicked = ImGui::IsItemClicked();
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f);
		anim.active        = approach(anim.active, ImGui::IsItemActive() ? 1.0f : 0.0f);
		anim.selected      = approach(anim.selected, active ? 1.0f : 0.0f);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 center(pos.x + diameter * 0.5f, pos.y + size.y * 0.5f);
		draw->AddCircle(center, diameter * 0.5f,
		                col_u32(Math::lerp(active_context()->style.colors.border, active_context()->style.colors.accent,
		                                   anim.hover + anim.selected)),
		                24, 2.0f);
		draw->AddCircleFilled(center, diameter * 0.31f * apply_ease(anim.selected, Ease::OutBack),
		                      col_u32(active_context()->style.colors.accent, anim.selected));
		draw_list_add_text(draw,
		                   ImVec2(pos.x + diameter + active_context()->style.spacing, pos.y + (size.y - text_size.y) * 0.5f),
		                   col_u32(active_context()->style.colors.text), visible_label(label));
		ImGui::PopID();
		return clicked;
	}

	bool radio_button(StringView label, int* value, int button_value)
	{
		const bool clicked = radio_button(label, value != nullptr && *value == button_value);
		if (clicked && value != nullptr && *value != button_value)
		{
			*value = button_value;
			return true;
		}
		return false;
	}

	bool segmented_control(StringView label, int* current_item, const char* const items[], int item_count, Size size_arg)
	{
		if (current_item == nullptr || items == nullptr || item_count <= 0)
		{
			return false;
		}
		cleanup_states();
		imgui_push_id(label);
		const Vec2 resolved_size = resolve(size_arg);
		const float width        = resolved_size.x > 0.0f ? resolved_size.x : ImGui::GetContentRegionAvail().x;
		const float height       = resolved_size.y > 0.0f ? resolved_size.y : active_context()->style.frame_height;
		const float segment_w    = width / static_cast<float>(item_count);
		const ImVec2 start       = ImGui::GetCursorScreenPos();
		ImDrawList* draw         = ImGui::GetWindowDrawList();
		draw->AddRectFilled(start, add(start, ImVec2(width, height)), col_u32(active_context()->style.colors.background),
		                    active_context()->style.rounding);
		draw->AddRect(start, add(start, ImVec2(width, height)), col_u32(active_context()->style.colors.border),
		              active_context()->style.rounding);

		bool changed = false;
		for (int i = 0; i < item_count; ++i)
		{
			ImGui::PushID(i);
			ImGui::SetCursorScreenPos(ImVec2(start.x + segment_w * i, start.y));
			ImGui::InvisibleButton("##segment", ImVec2(segment_w, height));
			const bool hovered = ImGui::IsItemHovered();
			const bool clicked = ImGui::IsItemClicked();
			AnimState& anim    = state_for(ImGui::GetID("segment_anim"));
			anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f);
			anim.selected      = approach(anim.selected, *current_item == i ? 1.0f : 0.0f);
			const ImVec2 min(start.x + segment_w * i, start.y);
			const ImVec2 max(start.x + segment_w * (i + 1), start.y + height);
			if (anim.hover > 0.01f)
			{
				draw->AddRectFilled(min, max, col_u32(active_context()->style.colors.background_hovered, anim.hover * 0.45f),
				                    i == 0 || i == item_count - 1 ? active_context()->style.rounding : 0.0f);
			}
			if (anim.selected > 0.01f)
			{
				draw->AddRectFilled(min, max, col_u32(active_context()->style.colors.accent, 0.28f * anim.selected),
				                    i == 0 || i == item_count - 1 ? active_context()->style.rounding : 0.0f);
				draw->AddRectFilled(ImVec2(min.x + 8.0f, max.y - 3.0f), ImVec2(max.x - 8.0f, max.y),
				                    col_u32(active_context()->style.colors.accent, anim.selected), 2.0f);
			}
			if (i > 0)
			{
				draw->AddLine(ImVec2(min.x, min.y + 6.0f), ImVec2(min.x, max.y - 6.0f),
				              col_u32(active_context()->style.colors.border));
			}
			const ImVec2 ts = ImGui::CalcTextSize(items[i]);
			draw->AddText(ImVec2(min.x + (segment_w - ts.x) * 0.5f, min.y + (height - ts.y) * 0.5f),
			              col_u32(active_context()->style.colors.text), items[i]);
			if (clicked && *current_item != i)
			{
				*current_item = i;
				changed       = true;
			}
			ImGui::PopID();
		}
		ImGui::SetCursorScreenPos(ImVec2(start.x, start.y + height));
		ImGui::Dummy(ImVec2(width, 0.0f));
		ImGui::PopID();
		return changed;
	}

	void progress_bar(float fraction, Size size_arg, StringView overlay)
	{
		cleanup_states();
		const ImGuiID id = ImGui::GetID("ui_progress_bar");
		AnimState& anim  = state_for(id);
		anim.value       = approach(anim.value, Math::clamp(fraction, 0.f, 1.f));
		ImVec2 size      = to_imvec(resolve(size_arg));
		if (size.x < 0.0f)
		{
			size.x = ImGui::GetContentRegionAvail().x;
		}
		if (size.y <= 0.0f)
		{
			size.y = 10.0f;
		}
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::Dummy(size);
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(pos, add(pos, size), col_u32(active_context()->style.colors.background_active), size.y * 0.5f);
		draw->AddRectFilled(pos, ImVec2(pos.x + size.x * anim.value, pos.y + size.y),
		                    col_u32(active_context()->style.colors.accent), size.y * 0.5f);
		if (!overlay.empty())
		{
			const ImVec2 ts = imgui_calc_text_size(overlay);
			draw_list_add_text(draw, ImVec2(pos.x + (size.x - ts.x) * 0.5f, pos.y + (size.y - ts.y) * 0.5f),
			                   col_u32(active_context()->style.colors.text), overlay);
		}
	}

	void spinner(StringView id, Unit radius, Unit thickness, const Vec4& color)
	{
		imgui_push_id(id);
		const float resolved_radius    = resolve(radius);
		const float resolved_thickness = resolve(thickness);
		const ImVec2 pos               = ImGui::GetCursorScreenPos();
		const ImVec2 size(resolved_radius * 2.0f + resolved_thickness * 2.0f, resolved_radius * 2.0f + resolved_thickness * 2.0f);
		ImGui::Dummy(size);
		ImDrawList* draw    = ImGui::GetWindowDrawList();
		const ImVec2 center = add(pos, mul(size, 0.5f));
		const float start   = static_cast<float>(ImGui::GetTime() * 6.0);
		const ImU32 c       = col_u32(has_color(color) ? color : active_context()->style.colors.accent);
		for (int i = 0; i < 24; ++i)
		{
			const float a0    = start + (static_cast<float>(i) / 24.0f) * 6.2831853f;
			const float a1    = start + (static_cast<float>(i + 1) / 24.0f) * 6.2831853f;
			const float alpha = static_cast<float>(i + 1) / 24.0f;
			draw->AddLine(ImVec2(center.x + Math::cos(a0) * resolved_radius, center.y + Math::sin(a0) * resolved_radius),
			              ImVec2(center.x + Math::cos(a1) * resolved_radius, center.y + Math::sin(a1) * resolved_radius),
			              col_u32(has_color(color) ? color : active_context()->style.colors.accent, alpha), resolved_thickness);
			(void) c;
		}
		ImGui::PopID();
	}

	bool color_edit(StringView label, Vec4* color, bool alpha, ColorEditFlags flags)
	{
		if (color == nullptr)
		{
			return false;
		}
		String label_storage(label);
		return alpha ? ImGui::ColorEdit4(label_storage.c_str(), &color->x, to_imgui_color_edit_flags(flags))
		             : ImGui::ColorEdit3(label_storage.c_str(), &color->x, to_imgui_color_edit_flags(flags));
	}

	bool keybind_input(StringView label, Keybind* binding)
	{
		if (binding == nullptr)
		{
			return false;
		}

		cleanup_states();
		imgui_push_id(label);
		const ImGuiID id        = ImGui::GetID("keybind");
		AnimState& anim         = state_for(id);
		const String display    = active_context()->keybind_capture == id ? "Press key..." : keybind_to_string(*binding);
		const ImVec2 label_size = imgui_calc_text_size(visible_label(label));
		const ImVec2 field_size(Math::max(130.0f, ImGui::CalcItemWidth() * 0.55f), active_context()->style.frame_height);
		ImGui::AlignTextToFramePadding();
		imgui_text_unformatted(visible_label(label));
		ImGui::SameLine();
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##keybind_button", field_size);
		const bool hovered = ImGui::IsItemHovered();
		if (ImGui::IsItemClicked())
		{
			active_context()->keybind_capture = id;
		}
		anim.hover       = approach(anim.hover, hovered ? 1.0f : 0.0f);
		anim.focus       = approach(anim.focus, active_context()->keybind_capture == id ? 1.0f : 0.0f);
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(pos, add(pos, field_size),
		                    col_u32(Math::lerp(active_context()->style.colors.background,
		                                       active_context()->style.colors.background_hovered, anim.hover)),
		                    active_context()->style.rounding);
		draw->AddRect(
		        pos, add(pos, field_size),
		        col_u32(Math::lerp(active_context()->style.colors.border, active_context()->style.colors.accent, anim.focus)),
		        active_context()->style.rounding, 0, active_context()->style.border_size);
		const ImVec2 ts = ImGui::CalcTextSize(display.c_str());
		draw->AddText(ImVec2(pos.x + active_context()->style.padding, pos.y + (field_size.y - ts.y) * 0.5f),
		              col_u32(active_context()->style.colors.text), display.c_str());
		(void) label_size;

		bool changed = false;

		if (active_context()->keybind_capture == id)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				active_context()->keybind_capture = 0;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Backspace) || ImGui::IsKeyPressed(ImGuiKey_Delete))
			{
				*binding                          = Keybind{};
				active_context()->keybind_capture = 0;
				changed                           = true;
			}
			else
			{
				for (int key_i = static_cast<int>(ui::Key::NamedKeyBegin); key_i < static_cast<int>(ui::Key::NamedKeyEnd);
				     ++key_i)
				{
					const ui::Key key = Key(static_cast<Key::Enum>(key_i));
					if (!is_modifier_key(key) && ImGui::IsKeyPressed(to_imgui_key(key), false))
					{
						ImGuiIO& io                       = ImGui::GetIO();
						binding->key_code                 = key;
						binding->ctrl                     = io.KeyCtrl;
						binding->shift                    = io.KeyShift;
						binding->alt                      = io.KeyAlt;
						binding->super                    = io.KeySuper;
						active_context()->keybind_capture = 0;
						changed                           = true;
						break;
					}
				}
			}
		}
		ImGui::PopID();
		return changed;
	}

	String keybind_to_string(const Keybind& binding)
	{
		if (binding.key_code == ui::Key::Undefined)
		{
			return "Undefined";
		}
		String result;
		if (binding.ctrl)
			result += "Ctrl+";
		if (binding.shift)
			result += "Shift+";
		if (binding.alt)
			result += "Alt+";
		if (binding.super)
			result += "Super+";
		result += ImGui::GetKeyName(to_imgui_key(binding.key_code));
		return result;
	}

	bool is_keybind_pressed(const Keybind& binding, bool repeat)
	{
		return binding.key_code != ui::Key::Undefined && keybind_mods_match(binding) &&
		       ImGui::IsKeyPressed(to_imgui_key(binding.key_code), repeat);
	}

}// namespace Trinex::UI
