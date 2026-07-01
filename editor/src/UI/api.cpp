#include "api_internal.hpp"
#include <Core/math/math.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_viewport.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>
#include <Window/window.hpp>

namespace Trinex::UI
{
	Context* g_context = nullptr;

	static void apply_imgui_style(const Style& value)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ColorTheme colors = value.colors;

		style.Alpha             = value.alpha;
		style.WindowPadding     = ImVec2(value.padding, value.padding);
		style.FramePadding      = ImVec2(value.padding, 6.0f);
		style.CellPadding       = ImVec2(value.padding, value.spacing);
		style.ItemSpacing       = ImVec2(value.spacing, value.spacing);
		style.WindowRounding    = value.rounding;
		style.ChildRounding     = value.rounding;
		style.PopupRounding     = value.rounding;
		style.FrameRounding     = value.rounding;
		style.ScrollbarRounding = value.rounding;
		style.GrabRounding      = value.rounding;
		style.TabRounding       = value.rounding;
		style.WindowBorderSize  = value.border_size;
		style.ChildBorderSize   = 0.0f;
		style.PopupBorderSize   = value.border_size;
		style.FrameBorderSize   = 0.0f;
		style.TabBorderSize     = 0.0f;
		style.AntiAliasedLines  = true;
		style.AntiAliasedFill   = true;

		ImVec4* imgui_colors                     = style.Colors;
		imgui_colors[ImGuiCol_Text]              = to_imvec(colors.text);
		imgui_colors[ImGuiCol_TextDisabled]      = to_imvec(colors.text_disabled);
		imgui_colors[ImGuiCol_WindowBg]          = to_imvec(colors.background);
		imgui_colors[ImGuiCol_ChildBg]           = ImVec4(0, 0, 0, 0);
		imgui_colors[ImGuiCol_PopupBg]           = to_imvec(colors.panel);
		imgui_colors[ImGuiCol_Border]            = to_imvec(colors.border);
		imgui_colors[ImGuiCol_BorderShadow]      = ImVec4(0, 0, 0, 0);
		imgui_colors[ImGuiCol_FrameBg]           = to_imvec(colors.background);
		imgui_colors[ImGuiCol_FrameBgHovered]    = to_imvec(colors.background_hovered);
		imgui_colors[ImGuiCol_FrameBgActive]     = to_imvec(colors.background_active);
		imgui_colors[ImGuiCol_TitleBg]           = to_imvec(Math::lerp(colors.background_active, colors.accent_active, 0.35f));
		imgui_colors[ImGuiCol_TitleBgActive]     = to_imvec(Math::lerp(colors.background_active, colors.accent, 0.24f));
		imgui_colors[ImGuiCol_TitleBgCollapsed]  = imgui_colors[ImGuiCol_TitleBg];
		imgui_colors[ImGuiCol_MenuBarBg]         = to_imvec(colors.panel);
		imgui_colors[ImGuiCol_CheckMark]         = to_imvec(colors.accent);
		imgui_colors[ImGuiCol_SliderGrab]        = to_imvec(colors.accent);
		imgui_colors[ImGuiCol_SliderGrabActive]  = to_imvec(colors.accent_active);
		imgui_colors[ImGuiCol_Button]            = ImVec4(0, 0, 0, 0);
		imgui_colors[ImGuiCol_ButtonHovered]     = to_imvec(with_alpha(colors.accent_hovered, 0.24f));
		imgui_colors[ImGuiCol_ButtonActive]      = to_imvec(with_alpha(colors.accent_active, 0.30f));
		imgui_colors[ImGuiCol_Header]            = to_imvec(with_alpha(colors.accent, 0.16f));
		imgui_colors[ImGuiCol_HeaderHovered]     = to_imvec(with_alpha(colors.accent_hovered, 0.24f));
		imgui_colors[ImGuiCol_HeaderActive]      = to_imvec(with_alpha(colors.accent_active, 0.30f));
		imgui_colors[ImGuiCol_Separator]         = to_imvec(colors.border);
		imgui_colors[ImGuiCol_TableHeaderBg]     = to_imvec(colors.background_active);
		imgui_colors[ImGuiCol_TableBorderStrong] = to_imvec(colors.border);
		imgui_colors[ImGuiCol_TableBorderLight]  = to_imvec(with_alpha(colors.border, 0.55f));
		imgui_colors[ImGuiCol_TableRowBg]        = to_imvec(with_alpha(colors.panel, 0.30f));
		imgui_colors[ImGuiCol_TableRowBgAlt]     = to_imvec(with_alpha(colors.background_hovered, 0.38f));
		imgui_colors[ImGuiCol_TextSelectedBg]    = to_imvec(with_alpha(colors.accent, 0.35f));
		imgui_colors[ImGuiCol_NavCursor]         = to_imvec(colors.accent);
	}

	/////////////////////// LIFECYCLE AND FRAME ///////////////////////

	Context* create_context(Trinex::Window* window)
	{
		Context* ctx = trx_new Context();
		ctx->window  = window;
		ctx->context = ImGui::CreateContext();
		ImGui::SetCurrentContext(ctx->context);
		ctx->style = Style{};

		ContextListener::for_each<&ContextListener::on_create>(ctx);

		auto& io = ImGui::GetIO();
		initialize_fonts(ctx);

		io.Fonts->Build();
		io.FontDefault = ctx->fonts[font_family_index(FontFamily::Default)][font_size_index(FontSize::Normal)];
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		apply_imgui_style(ctx->style);

		// Begin window hook
		{
			ImGuiContextHook hook;
			hook.Type     = ImGuiContextHookType_BeginWindowPost;
			hook.UserData = nullptr;
			hook.Owner    = 0;
			hook.Callback = +[](ImGuiContext* ctx, ImGuiContextHook* hook) {
				if (auto render_scale = active_context()->render_scale)
				{
					ImGuiWindow* window = ImGui::GetCurrentWindow();
					render_scale->scope = trx_stack_new RenderScale::Scope(window->DrawList, 0, 0, render_scale->scope);

					ImRect rect       = ImGui::GetCurrentWindow()->Rect();
					render_scale->min = Math::min(render_scale->min, to_vec(rect.Min));
					render_scale->max = Math::max(render_scale->max, to_vec(rect.Max));
				}
			};

			ImGui::AddContextHook(ctx->context, &hook);
		}

		register_console_commands(ctx);
		return ctx;
	}

	void destroy_context(Context* context)
	{
		ImGuiContext* ctx = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(context->context);

		ContextListener::reverse_for_each<&ContextListener::on_destroy>(context);

		ImGui::SetCurrentContext(ctx);
		ImGui::DestroyContext(context->context);
		trx_delete context;
	}

	bool begin_frame(Context* context)
	{
		trinex_assert(context);
		g_context = context;

		ImGui::SetCurrentContext(context->context);

		ContextListener::for_each<&ContextListener::on_begin_frame>(context);

		ImGui::NewFrame();
		context->stack_memory_location = StackByteAllocator::location();

		return true;
	}

	void end_frame()
	{
		trinex_assert(g_context);

		// Render widgets
		{
			PersistentWindow** list = &active_context()->window_list;

			while (*list)
			{
				PersistentWindow* window = *list;
				Widget* widget           = window->widget;

				bool open = widget->is_open();

				if (open)
				{
					const bool visible = begin_window(widget->name().c_str(), &open, widget->options());

					if (ImGui::IsWindowAppearing())
					{
						widget->on_open();
					}

					if (visible)
					{
						window->widget->on_render();
						open = open && widget->is_open();
						end_window();
					}

					widget->is_open(open);
				}

				list = &window->next;
			}
		}

		// Render notifications

		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			const ImVec2 origin(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f,
			                    viewport->WorkPos.y + viewport->WorkSize.y - 16.0f);
			float target_y   = origin.y;
			ImDrawList* draw = ImGui::GetForegroundDrawList(viewport);

			for (auto it = active_context()->notifications.begin(); it != active_context()->notifications.end();)
			{
				it->age += dt();
				const float in_t   = Math::clamp(it->age / 0.25f, 0.f, 1.f);
				const float out_t  = Math::clamp((it->duration - it->age) / 0.35f, 0.f, 1.f);
				const float alpha  = Math::clamp(Math::min(in_t, out_t), 0.f, 1.f);
				const float slide  = (1.0f - apply_ease(in_t)) * 24.0f;
				const float width  = 320.0f;
				const float height = (it->title.empty() ? 52.0f : 70.0f) + (it->action_label.empty() ? 0.0f : 28.0f);
				float& animated_y  = active_context()->notification_y[it->id];
				if (animated_y == 0.0f)
				{
					animated_y = target_y;
				}
				animated_y = approach(animated_y, target_y);
				const ImVec2 max(origin.x - slide, animated_y);
				const ImVec2 min(max.x - width, animated_y - height);
				const Vec4 accent = notification_color(it->kind);

				draw->AddRectFilled(min, max, col_u32(active_context()->style.colors.panel, alpha * 0.96f),
				                    active_context()->style.rounding);
				draw->AddRect(min, max, col_u32(active_context()->style.colors.border, alpha), active_context()->style.rounding);
				draw->AddRectFilled(min, ImVec2(min.x + 4.0f, max.y), col_u32(accent, alpha), active_context()->style.rounding,
				                    ImDrawFlags_RoundCornersLeft);
				float tx = min.x + 16.0f;
				float ty = min.y + 12.0f;
				if (!it->title.empty())
				{
					draw->AddText(ImVec2(tx, ty), col_u32(active_context()->style.colors.text, alpha), it->title.c_str());
					ty += ImGui::GetTextLineHeight() + 5.0f;
				}
				draw->AddText(ImVec2(tx, ty), col_u32(active_context()->style.colors.text_muted, alpha), it->message.c_str());
				if (!it->action_label.empty())
				{
					const ImVec2 button_size(ImGui::CalcTextSize(it->action_label.c_str()).x + 18.0f, 22.0f);
					const ImVec2 button_min(max.x - button_size.x - 12.0f, max.y - button_size.y - 10.0f);
					const ImVec2 button_max = add(button_min, button_size);
					const bool hovered      = ImGui::IsMouseHoveringRect(button_min, button_max);
					draw->AddRectFilled(button_min, button_max,
					                    col_u32(hovered ? active_context()->style.colors.accent_hovered
					                                    : active_context()->style.colors.accent,
					                            alpha),
					                    5.0f);
					draw->AddText(ImVec2(button_min.x + 9.0f, button_min.y + 3.0f),
					              col_u32(active_context()->style.colors.text, alpha), it->action_label.c_str());

					if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						if (it->action)
						{
							it->action();
						}
						active_context()->notification_y.erase(it->id);
						it = active_context()->notifications.erase(it);
						continue;
					}
				}
				target_y += -(height + 10.0f);
				if (it->age > it->duration)
				{
					active_context()->notification_y.erase(it->id);
					it = active_context()->notifications.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		ContextListener::for_each<&ContextListener::on_end_frame>(g_context);

		ImGui::Render();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
		}

		ContextListener::for_each<&ContextListener::on_render>(g_context);

		{
			auto& stack = active_context()->shadow_stack;
			trinex_assert_msg(stack.empty(), "UI::push_shadow()/pop_shadow() imbalance detected at end_frame()");
			if (!stack.empty())
			{
				stack.clear();
			}
		}

		{
			auto& stack = active_context()->glass_panel_stack;

			trinex_assert(stack.empty() && "UI::begin_glass_panel()/end_glass_panel() imbalance detected at end_frame()");
			if (!stack.empty())
			{
				stack.clear();
			}
		}

		StackByteAllocator::location(g_context->stack_memory_location);

		g_context = nullptr;
		ImGui::SetCurrentContext(nullptr);
	}

	void push_font(FontFamily family, FontSize size)
	{
		if (ImFont* value = resolve_font(active_context(), family, size))
		{
			ImGui::PushFont(value);
		}
	}

	void pop_font()
	{
		ImGui::PopFont();
	}

	void push_text_font(FontSize size)
	{
		push_font(FontFamily::Text, size);
	}

	void push_icons_font(FontSize size)
	{
		push_font(FontFamily::Icons, size);
	}

	/////////////////////// UNITS ///////////////////////

	Unit px(float value)
	{
		return Unit{Unit::Px, value};
	}

	Unit dp(float value)
	{
		return Unit{Unit::Dp, value};
	}

	Unit rem(float value)
	{
		return Unit{Unit::Rem, value};
	}

	Unit percent(float value)
	{
		return Unit{Unit::Percent, value};
	}

	Unit fill()
	{
		return Unit{Unit::Fill, 1.0f};
	}

	Size size(Unit width, Unit height)
	{
		return Size{width, height};
	}

	Size px(float width, float height)
	{
		return size(px(width), px(height));
	}

	Size dp(float width, float height)
	{
		return size(dp(width), dp(height));
	}

	Size rem(float width, float height)
	{
		return size(rem(width), rem(height));
	}

	Size percent(float width, float height)
	{
		return size(percent(width), percent(height));
	}

	Size fill_size()
	{
		return size(fill(), fill());
	}

	float resolve(Unit value, Axis axis)
	{
		switch (value.type)
		{
			case Unit::Px:
			{
				const ImVec2 scale  = ImGui::GetIO().DisplayFramebufferScale;
				const float divisor = axis == Axis::Y ? scale.y : scale.x;
				return divisor > 0.0f ? value.value / divisor : value.value;
			}
			case Unit::Rem: return value.value * ImGui::GetFontSize();
			case Unit::Percent:
			{
				const ImVec2 available = ImGui::GetContentRegionAvail();
				return value.value * (axis == Axis::Y ? available.y : available.x);
			}
			case Unit::Fill:
			{
				const ImVec2 available = ImGui::GetContentRegionAvail();
				return axis == Axis::Y ? available.y : available.x;
			}
			case Unit::Dp:
			default: return value.value;
		}
	}

	Vec2 resolve(const Size& value)
	{
		return Vec2(resolve(value.width, Axis::X), resolve(value.height, Axis::Y));
	}

	/////////////////////// STYLE AND EFFECTS ///////////////////////

	Style& style()
	{
		return active_context()->style;
	}

	void style(const Style& value)
	{
		active_context()->style = value;
		apply_imgui_style(value);
	}

	void push_style(const Style& value)
	{
		active_context()->style_stack.push_back(active_context()->style);
		active_context()->style = value;
		apply_imgui_style(value);
	}

	void pop_style()
	{
		if (!active_context()->style_stack.empty())
		{
			active_context()->style = active_context()->style_stack.back();
			active_context()->style_stack.pop_back();
			apply_imgui_style(active_context()->style);
		}
	}

	void push_style_color(StyleColor color, const Vec4& value)
	{
		ImGui::PushStyleColor(static_cast<ImGuiCol>(color.value), to_imvec(value));
	}

	void pop_style_color(u32 count)
	{
		ImGui::PopStyleColor(count);
	}

	void push_style_var(StyleVar var, f32 value)
	{
		ImGui::PushStyleVar(static_cast<ImGuiStyleVar>(var.value), value);
	}

	void push_style_var(StyleVar var, const Vec2& value)
	{
		ImGui::PushStyleVar(static_cast<ImGuiStyleVar>(var.value), to_imvec(value));
	}

	void pop_style_var(u32 count)
	{
		ImGui::PopStyleVar(count);
	}

	void push_shadow(const ShadowOptions& shadow)
	{
		active_context()->shadow_stack.push_back(shadow);
	}

	void pop_shadow()
	{
		auto& stack = active_context()->shadow_stack;

		trinex_assert(!stack.empty() && "UI::pop_shadow() called without matching push_shadow()");

		if (!stack.empty())
		{
			stack.pop_back();
		}
	}

	void blur(const Vec2& min, const Vec2& max, DrawList draw_list, const BlurOptions& options)
	{
		const float spread = Math::max(0.0f, options.spread);
		const Vec2 pad(spread, spread);
		const Vec2 area_min  = min - pad;
		const Vec2 area_max  = max + pad;
		const Vec2 area_size = area_max - area_min;

		if (options.radius > 0.0f)
		{
			paint(PaintOptions{.draw_list = draw_list}, [options, area_min, area_max](RHIContext* ctx, RHITexture* layer) {
				const float radius = Math::clamp(options.radius, 0.0f, 64.0f);

				if (radius <= 0.0f)
				{
					return;
				}

				const float sigma           = options.sigma > 0.0f ? options.sigma : Math::max(1.0f, radius * 0.45f);
				const RHITextureFlags flags = RHITextureFlags::ColorAttachment;
				RHITexturePool* pool        = RHITexturePool::global_instance();

				const Vector2u viewport_size = layer->size();
				RHITexture* temporary        = pool->acquire(RHISurfaceFormat::RGBA8, viewport_size, flags);

				const Vector2f blur_offset = area_min / Vector2f(viewport_size);
				const Vector2f blur_size   = (area_max - area_min) / Vector2f(viewport_size);

				ctx->push_debug_stage("Bloor");
				{
					ctx->barrier(layer, RHIAccess::SRVGraphics);
					ctx->barrier(temporary, RHIAccess::RTV);

					ctx->begin_rendering(temporary->as_rtv());
					Pipelines::GaussianBlur::blur(ctx, layer->as_srv(), {0.f, 1.f / static_cast<f32>(viewport_size.y)}, sigma,
					                              radius, {}, nullptr, blur_offset, blur_size);
					ctx->end_rendering();

					ctx->barrier(layer, RHIAccess::RTV);
					ctx->barrier(temporary, RHIAccess::SRVGraphics);
					ctx->begin_rendering(layer->as_rtv());

					Pipelines::GaussianBlur::blur(ctx, temporary->as_srv(), {1.f / static_cast<f32>(viewport_size.x), 0.f}, sigma,
					                              radius, {}, nullptr, blur_offset, blur_size);

					if (options.noise_opacity > 0.f)
					{
						ctx->push_debug_stage("Noise");
						Pipelines::NoiseApplication::noise(ctx, options.noise_opacity, options.noise_scale, blur_offset,
						                                   blur_size);
						ctx->pop_debug_stage();
					}

					ctx->end_rendering();
				}
				ctx->pop_debug_stage();
				pool->release(temporary);
			});
		}

		if (options.tint.w > 0.0f)
		{
			ImGuiWindow* window     = ImGui::GetCurrentWindow();
			ImGuiViewport* viewport = nullptr;
			ImDrawList* list        = resolve_draw_list(draw_list, window, viewport);
			if (list != nullptr)
			{
				const float base_rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
				list->AddRectFilled(to_imvec(area_min), to_imvec(area_max), col_u32(options.tint),
				                    Math::max(0.0f, base_rounding + spread));
			}
		}
	}

	void push_render_scale(Vec2 scale, Vec2 pivot, RenderScaleFlags flags)
	{
		Context* context = active_context();

		RenderScale* render_scale = context->stack.push<RenderScale>();
		render_scale->prev        = context->render_scale;
		context->render_scale     = render_scale;

		render_scale->scope = trx_stack_new RenderScale::Scope(ImGui::GetWindowDrawList());
		render_scale->scale = scale;
		render_scale->pivot = pivot;

		if (flags & RenderScaleFlags::StartFromLastItemBounds)
		{
			render_scale->min = item_rect_min();
			render_scale->max = item_rect_max();
		}
		else
		{
			render_scale->min = cursor_screen_position();
			render_scale->max = render_scale->min;
		}
	}

	void pop_render_scale()
	{
		Context* context = active_context();

		RenderScale* render_scale = context->stack.pop<RenderScale>();
		trinex_assert(render_scale && context->render_scale == render_scale &&
		              "UI::pop_render_scale() called without matching push_render_scale()");

		context->render_scale = render_scale->prev;

		if (render_scale->scale.x != 1.f || render_scale->scale.y != 1.f)
		{
			// Update bounds
			{
				render_scale->max = Math::max(render_scale->max, item_rect_max());
			}

			// Scale content
			for (auto scope = render_scale->scope; scope; scope = scope->next)
			{
				ImDrawList* draw_list = scope->draw_list;
				i32 vtx_end           = draw_list->VtxBuffer.Size;
				i32 cmd_end           = draw_list->CmdBuffer.Size;

				if (vtx_end <= scope->start_vertex)
					continue;

				const Vec2 pivot = render_scale->min + (render_scale->max - render_scale->min) * render_scale->pivot;

				for (u32 i = scope->start_vertex; i < vtx_end; ++i)
				{
					ImVec2& p = draw_list->VtxBuffer[i].pos;

					p.x = pivot.x + (p.x - pivot.x) * render_scale->scale.x;
					p.y = pivot.y + (p.y - pivot.y) * render_scale->scale.y;
				}

				for (u32 i = scope->start_command; i < cmd_end; ++i)
				{
					ImVec4& rect = draw_list->CmdBuffer[i].ClipRect;

					rect.x = pivot.x + (rect.x - pivot.x) * render_scale->scale.x;
					rect.y = pivot.y + (rect.y - pivot.y) * render_scale->scale.y;

					rect.z = pivot.x + (rect.z - pivot.x) * render_scale->scale.x;
					rect.w = pivot.y + (rect.w - pivot.y) * render_scale->scale.y;
				}
			}
		}
	}

	/////////////////////// ANIMATION AND IDENTITY ///////////////////////

	float apply_ease(float t, Ease mode)
	{
		t = Math::clamp(t, 0.0f, 1.0f);

		switch (mode)
		{
			case Ease::Linear:
			{
				return t;
			}

			case Ease::InQuad:
			{
				return t * t;
			}

			case Ease::OutQuad:
			{
				const float inv = 1.0f - t;
				return 1.0f - inv * inv;
			}

			case Ease::InOutQuad:
			{
				if (t < 0.5f)
					return 2.0f * t * t;

				const float v = -2.0f * t + 2.0f;
				return 1.0f - (v * v) * 0.5f;
			}

			case Ease::OutCubic:
			{
				const float inv = 1.0f - t;
				return 1.0f - inv * inv * inv;
			}

			case Ease::InExpo:
			{
				if (t <= 0.0f)
					return 0.0f;

				return Math::pow(2.0f, 10.0f * t - 10.0f);
			}

			case Ease::OutExpo:
			{
				if (t >= 1.0f)
					return 1.0f;

				return 1.0f - Math::pow(2.0f, -10.0f * t);
			}

			case Ease::InOutExpo:
			{
				if (t <= 0.0f)
					return 0.0f;

				if (t >= 1.0f)
					return 1.0f;

				if (t < 0.5f)
					return Math::pow(2.0f, 20.0f * t - 10.0f) * 0.5f;

				return (2.0f - Math::pow(2.0f, -20.0f * t + 10.0f)) * 0.5f;
			}

			case Ease::OutBack:
			{
				const float c1 = 1.70158f;
				const float c3 = c1 + 1.0f;

				const float v = t - 1.0f;
				return 1.0f + c3 * v * v * v + c1 * v * v;
			}

			default:
			{
				return t;
			}
		}
	}

	float animate_float(ID id, float target, float speed)
	{
		AnimState& anim = state_for(to_imgui_id(id));
		anim.value      = approach(anim.value, target, speed < 0.0f ? active_context()->style.animation_speed : speed);
		return anim.value;
	}

	Vec2 animate_vec2(ID id, const Vec2& target, float speed)
	{
		AnimState& anim = state_for(to_imgui_id(id));
		const float s   = speed < 0.0f ? active_context()->style.animation_speed : speed;
		anim.value      = approach(anim.value, target.x, s);
		anim.extra      = approach(anim.extra, target.y, s);
		return Vec2(anim.value, anim.extra);
	}

	Vec4 animate_color(ID id, const Vec4& target, float speed)
	{
		AnimState& anim = state_for(to_imgui_id(id));
		const float s   = speed < 0.0f ? active_context()->style.animation_speed : speed;
		anim.hover      = approach(anim.hover, target.x, s);
		anim.active     = approach(anim.active, target.y, s);
		anim.focus      = approach(anim.focus, target.z, s);
		anim.selected   = approach(anim.selected, target.w, s);
		return Vec4(anim.hover, anim.active, anim.focus, anim.selected);
	}

	void reset_animation(ID id)
	{
		active_context()->anim.erase(to_imgui_id(id));
	}

	void clear_animations()
	{
		active_context()->anim.clear();
	}

	void push_id(StringView id)
	{
		ImGui::PushID(id.data(), id.data() + id.size());
	}

	void push_id(i32 id)
	{
		ImGui::PushID(id);
	}

	void push_id(const void* ptr)
	{
		ImGui::PushID(ptr);
	}

	void pop_id()
	{
		ImGui::PopID();
	}

	ID id(StringView id)
	{
		return to_ui_id(ImGui::GetID(id.data(), id.data() + id.size()));
	}

	/////////////////////// DOCKING ///////////////////////

	bool is_window_docked()
	{
		return ImGui::IsWindowDocked();
	}

	ID window_dock_id()
	{
		return to_ui_id(ImGui::GetWindowDockID());
	}

	void undock_window()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindowRead();
		if (window != nullptr && window->DockNode != nullptr)
		{
			ImGui::DockContextQueueUndockWindow(ImGui::GetCurrentContext(), window);
		}
	}

	static void dock_builder_begin(ID dockspace_id, const Size& size, DockNodeFlags flags)
	{
		const ImGuiID root_id      = to_imgui_id(dockspace_id);
		ImGuiViewport* viewport    = ImGui::GetMainViewport();
		const ImVec2 fallback_size = viewport != nullptr ? viewport->WorkSize : ImVec2(0.0f, 0.0f);
		const Vec2 requested       = resolve(size);
		const ImVec2 resolved_size(requested.x > 0.0f ? requested.x : fallback_size.x,
		                           requested.y > 0.0f ? requested.y : fallback_size.y);

		ImGui::DockBuilderRemoveNode(root_id);
		ImGui::DockBuilderAddNode(root_id, to_imgui_dock_node_flags(flags) | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(root_id, resolved_size);
	}

	static void dock_builder_set_flags(ID dock_id, DockNodeFlags flags)
	{
		if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(to_imgui_id(dock_id)))
		{
			node->SetLocalFlags(to_imgui_dock_node_flags(flags));
		}
	}

	static DockBuilderSplitResult dock_builder_split(ID dock, DockSplitDir dir, float ratio)
	{
		DockBuilderSplitResult result;
		ImGuiID remainder = 0;
		ImGuiID child     = 0;

		ImGui::DockBuilderSplitNode(to_imgui_id(dock), to_imgui_dock_dir(dir), Math::clamp(ratio, 0.0f, 1.0f), &child,
		                            &remainder);

		result.remainder = to_ui_id(remainder);
		result.child     = to_ui_id(child);
		return result;
	}

	static void dock_builder_dock_window(StringView window_name, ID dock_id)
	{
		String storage(window_name);
		ImGui::DockBuilderDockWindow(storage.c_str(), to_imgui_id(dock_id));
	}

	bool DockLayout::exists() const
	{
		return ImGui::DockBuilderGetNode(to_imgui_id(m_root)) != nullptr;
	}

	DockLayout& DockLayout::bind(StringView id, DockID dock)
	{
		if (!has_text(id))
		{
			return *this;
		}

		for (NamedDock& named : m_named)
		{
			if (named.id == id)
			{
				trinex_assert(!named.dock || named.dock == dock);
				named.dock = dock;
				return *this;
			}
		}

		NamedDock& named = m_named.emplace_back();
		named.id         = id;
		named.dock       = dock;
		return *this;
	}

	DockID DockLayout::find(StringView id) const
	{
		if (!has_text(id))
		{
			return DockID();
		}

		for (const NamedDock& named : m_named)
		{
			if (named.id == id)
			{
				return named.dock;
			}
		}

		return DockID();
	}

	DockID DockLayout::require(StringView id) const
	{
		const DockID dock = find(id);
		trinex_assert(dock && "UI::DockLayout::require() cannot find named dock");
		return dock;
	}

	bool DockLayout::has(StringView id) const
	{
		return static_cast<bool>(find(id));
	}

	DockLayout& DockLayout::flags(DockID dock_id, DockNodeFlags flags)
	{
		dock_builder_set_flags(dock_id, flags);
		return *this;
	}

	DockLayout& DockLayout::flags(StringView id, DockNodeFlags flags)
	{
		return this->flags(require(id), flags);
	}

	DockBuilderSplitResult DockLayout::split(DockID dock, DockSplitDir dir, float ratio, StringView id)
	{
		DockBuilderSplitResult result = dock_builder_split(dock, dir, ratio);
		bind(id, result.child);
		return result;
	}

	DockBuilderSplitResult DockLayout::split(DockID dock, DockSplitDir dir, float ratio, StringView remainder_id,
	                                         StringView child_id)
	{
		DockBuilderSplitResult result = dock_builder_split(dock, dir, ratio);

		if (dock == m_main)
		{
			m_main = result.remainder;
		}

		bind(remainder_id, result.remainder);
		bind(child_id, result.child);
		return result;
	}

	DockID DockLayout::crop(DockID& dock, DockSplitDir dir, float ratio, StringView id)
	{
		auto result = split(dock, dir, ratio, id);
		dock        = result.remainder;
		return result.child;
	}

	DockID DockLayout::crop(DockID& dock, DockSplitDir dir, float ratio, StringView remainder_id, StringView child_id)
	{
		auto result = split(dock, dir, ratio, remainder_id, child_id);
		dock        = result.remainder;
		return result.child;
	}

	DockID DockLayout::dock(StringView window_name, DockID dock_id)
	{
		dock_builder_dock_window(window_name, dock_id);
		return dock_id;
	}

	DockID DockLayout::dock(StringView window_name, StringView dock_id)
	{
		return dock(window_name, require(dock_id));
	}

	bool DockLayout::begin(Size size, DockNodeFlags flags)
	{
		const ImGuiID root = ImGui::GetID("##dockspace");

		m_root = ID(root);
		m_main = ID(root);
		m_named.clear();

		const ImVec2 fallback_size = ImGui::GetWindowSize();
		const Vec2 requested       = resolve(size);
		const ImVec2 resolved_size(requested.x > 0.0f ? requested.x : fallback_size.x,
		                           requested.y > 0.0f ? requested.y : fallback_size.y);

		ImGui::DockBuilderRemoveNode(root);
		ImGui::DockBuilderAddNode(root, to_imgui_dock_node_flags(flags) | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(root, resolved_size);
		return true;
	}

	bool DockLayout::begin(DockID root_id, Size size, DockNodeFlags flags)
	{
		if (!root_id)
		{
			return begin(size, flags);
		}

		const ImGuiID root = to_imgui_id(root_id);

		m_root = root_id;
		m_main = root_id;
		m_named.clear();

		const ImVec2 fallback_size =
		        ImGui::GetMainViewport() != nullptr ? ImGui::GetMainViewport()->WorkSize : ImVec2(0.0f, 0.0f);
		const Vec2 requested = resolve(size);
		const ImVec2 resolved_size(requested.x > 0.0f ? requested.x : fallback_size.x,
		                           requested.y > 0.0f ? requested.y : fallback_size.y);

		ImGui::DockBuilderRemoveNode(root);
		ImGui::DockBuilderAddNode(root, to_imgui_dock_node_flags(flags) | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(root, resolved_size);
		return true;
	}

	DockLayout& DockLayout::end()
	{
		ImGui::DockBuilderFinish(to_imgui_id(m_root));
		return *this;
	}

	bool begin_dockspace(const DockLayoutOptions& options)
	{
		ImGuiID id = options.id ? to_imgui_id(options.id) : ImGui::GetID("##dockspace");

		if (!(id = ImGui::DockSpace(id, to_imvec(resolve(options.size)), to_imgui_dock_node_flags(options.flags))))
			return false;

		return ImGui::IsWindowAppearing() || options.reset;
	}

	void end_docspace()
	{
		// Dummy function. Do nothing
	}

	bool begin_viewport_dockspace(const DockLayoutOptions& options)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		if (viewport == nullptr)
		{
			return false;
		}

		ImGuiID id = options.id ? to_imgui_id(options.id) : ImGui::GetID("##dockspace");
		ImGui::DockSpaceOverViewport(id, viewport, to_imgui_dock_node_flags(options.flags));
		return ImGui::IsWindowAppearing() || options.reset;
	}

	void end_viewport_dockspace()
	{
		// Dummy function. Do nothing
	}

	void dockspace(const DockLayoutOptions& options, const FunctionRef<void(DockLayout&)>& function)
	{
		if (begin_dockspace(options))
		{
			DockLayout builder;
			if (builder.begin(options.id, options.size, options.flags))
			{
				function(builder);
				builder.end();
			}
			end_docspace();
		}
	}

	void dockspace(const FunctionRef<void(DockLayout&)>& builder)
	{
		dockspace({}, builder);
	}

	void viewport_dockspace(const DockLayoutOptions& options, const FunctionRef<void(DockLayout&)>& function)
	{
		if (begin_viewport_dockspace(options))
		{
			DockLayout builder;
			if (builder.begin(options.id, options.size, options.flags))
			{
				function(builder);
				builder.end();
			}
			end_viewport_dockspace();
		}
	}

	void viewport_dockspace(const FunctionRef<void(DockLayout&)>& builder)
	{
		viewport_dockspace({}, builder);
	}

}// namespace Trinex::UI
