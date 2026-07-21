#include "internal.hpp"
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

	static void initialize_style(ImGuiStyle* style)
	{
		if (!style)
			return;

		ImVec4* colors = style->Colors;

		constexpr float deg_to_rad = IM_PI / 180.0f;

		// ============================================================================
		// Palette
		// ============================================================================

		const ImVec4 text       = ImVec4(0.86f, 0.88f, 0.91f, 1.00f);
		const ImVec4 text_muted = ImVec4(0.48f, 0.51f, 0.56f, 1.00f);
		const ImVec4 text_dim   = ImVec4(0.34f, 0.36f, 0.40f, 1.00f);

		const ImVec4 bg_0 = ImVec4(0.035f, 0.037f, 0.042f, 1.00f);// main bg
		const ImVec4 bg_1 = ImVec4(0.050f, 0.053f, 0.060f, 1.00f);// panels
		const ImVec4 bg_2 = ImVec4(0.070f, 0.075f, 0.085f, 1.00f);// frames
		const ImVec4 bg_3 = ImVec4(0.095f, 0.103f, 0.116f, 1.00f);// hover
		const ImVec4 bg_4 = ImVec4(0.125f, 0.138f, 0.155f, 1.00f);// active
		const ImVec4 bg_5 = ImVec4(0.160f, 0.175f, 0.195f, 1.00f);// strong active

		const ImVec4 border       = ImVec4(0.155f, 0.165f, 0.185f, 0.90f);
		const ImVec4 border_soft  = ImVec4(0.120f, 0.130f, 0.145f, 0.70f);
		const ImVec4 border_focus = ImVec4(0.360f, 0.520f, 0.640f, 0.80f);

		const ImVec4 accent       = ImVec4(0.330f, 0.520f, 0.640f, 1.00f);
		const ImVec4 accent_hover = ImVec4(0.420f, 0.620f, 0.740f, 1.00f);
		const ImVec4 accent_hot   = ImVec4(0.540f, 0.760f, 0.900f, 1.00f);

		// ============================================================================
		// Global
		// ============================================================================

		style->Alpha         = 1.0f;
		style->DisabledAlpha = 0.45f;

		// ============================================================================
		// Window
		// ============================================================================

		style->WindowPadding    = ImVec2(10.0f, 8.0f);
		style->WindowMinSize    = ImVec2(64.0f, 64.0f);
		style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style->WindowRounding   = 6.0f;
		style->WindowBorderSize = 1.0f;

		// ============================================================================
		// Child Window
		// ============================================================================

		style->ChildRounding   = 5.0f;
		style->ChildBorderSize = 1.0f;

		// ============================================================================
		// Popup / Tooltip / Menu Popup
		// ============================================================================

		style->PopupRounding   = 6.0f;
		style->PopupBorderSize = 1.0f;

		// ============================================================================
		// Layout
		// ============================================================================

		style->ItemSpacing      = ImVec2(8.0f, 5.0f);
		style->ItemInnerSpacing = ImVec2(6.0f, 4.0f);
		style->IndentSpacing    = 18.0f;

		// ============================================================================
		// Frame
		// ============================================================================

		style->FramePadding    = ImVec2(8.0f, 4.0f);
		style->FrameRounding   = 4.0f;
		style->FrameBorderSize = 1.0f;

		// ============================================================================
		// Button
		// ============================================================================

		style->ButtonTextAlign = ImVec2(0.5f, 0.5f);

		// ============================================================================
		// Header / Selectable / TreeNode / MenuItem
		// ============================================================================

		style->SelectableTextAlign = ImVec2(0.0f, 0.5f);

		// ============================================================================
		// Scrollbar
		// ============================================================================

		style->ScrollbarSize     = 13.0f;
		style->ScrollbarRounding = 6.0f;

		// ============================================================================
		// Slider / Drag Grab
		// ============================================================================

		style->GrabMinSize  = 11.0f;
		style->GrabRounding = 4.0f;

		// ============================================================================
		// Separator
		// ============================================================================

		style->SeparatorTextBorderSize = 1.0f;
		style->SeparatorTextAlign      = ImVec2(0.0f, 0.5f);
		style->SeparatorTextPadding    = ImVec2(16.0f, 4.0f);

		// ============================================================================
		// Tab
		// ============================================================================

		style->TabRounding        = 4.0f;
		style->TabBorderSize      = 0.0f;
		style->TabMinWidthBase    = 44.0f;
		style->TabMinWidthShrink  = 24.0f;
		style->TabBarBorderSize   = 1.0f;
		style->TabBarOverlineSize = 2.0f;

		// ============================================================================
		// Table
		// ============================================================================

		style->CellPadding                 = ImVec2(8.0f, 4.0f);
		style->TableAngledHeadersAngle     = 35.0f * deg_to_rad;
		style->TableAngledHeadersTextAlign = ImVec2(0.5f, 0.0f);

		// ============================================================================
		// Tree
		// ============================================================================

		style->TreeLinesSize     = 1.25f;
		style->TreeLinesRounding = 3.0f;

		// ============================================================================
		// Docking
		// ============================================================================

		style->DockingSeparatorSize = 2.0f;

		// ============================================================================
		// Image
		// ============================================================================

		style->ImageBorderSize = 1.0f;

		// ============================================================================
		// Text
		// ============================================================================

		colors[ImGuiCol_Text]            = text;
		colors[ImGuiCol_TextDisabled]    = text_muted;
		colors[ImGuiCol_TextLink]        = ImVec4(0.480f, 0.680f, 0.800f, 1.00f);
		colors[ImGuiCol_TextSelectedBg]  = ImVec4(0.330f, 0.520f, 0.640f, 0.35f);
		colors[ImGuiCol_InputTextCursor] = accent_hot;

		// ============================================================================
		// Window
		// ============================================================================

		colors[ImGuiCol_WindowBg]          = bg_0;
		colors[ImGuiCol_Border]            = border;
		colors[ImGuiCol_BorderShadow]      = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
		colors[ImGuiCol_TitleBg]           = ImVec4(0.040f, 0.042f, 0.048f, 1.00f);
		colors[ImGuiCol_TitleBgActive]     = ImVec4(0.060f, 0.066f, 0.076f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]  = ImVec4(0.030f, 0.032f, 0.037f, 1.00f);
		colors[ImGuiCol_MenuBarBg]         = bg_1;
		colors[ImGuiCol_ResizeGrip]        = ImVec4(0.330f, 0.520f, 0.640f, 0.22f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.420f, 0.620f, 0.740f, 0.45f);
		colors[ImGuiCol_ResizeGripActive]  = ImVec4(0.540f, 0.760f, 0.900f, 0.75f);

		// ============================================================================
		// Child Window
		// ============================================================================

		colors[ImGuiCol_ChildBg] = bg_1;

		// ============================================================================
		// Popup / Tooltip / Menu Popup
		// ============================================================================

		colors[ImGuiCol_PopupBg] = ImVec4(0.045f, 0.048f, 0.055f, 1.00f);

		// ============================================================================
		// Frame
		// ============================================================================

		colors[ImGuiCol_FrameBg]        = bg_2;
		colors[ImGuiCol_FrameBgHovered] = bg_3;
		colors[ImGuiCol_FrameBgActive]  = bg_4;

		// ============================================================================
		// Button
		// ============================================================================

		colors[ImGuiCol_Button]        = ImVec4(0.075f, 0.082f, 0.094f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.105f, 0.118f, 0.135f, 1.00f);
		colors[ImGuiCol_ButtonActive]  = ImVec4(0.145f, 0.165f, 0.190f, 1.00f);

		// ============================================================================
		// Checkbox / Radio / Markers
		// ============================================================================

		colors[ImGuiCol_CheckMark] = accent_hot;

		// ============================================================================
		// Header / Selectable / TreeNode / MenuItem
		// ============================================================================

		colors[ImGuiCol_Header]        = ImVec4(0.080f, 0.090f, 0.105f, 0.90f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.115f, 0.132f, 0.152f, 0.95f);
		colors[ImGuiCol_HeaderActive]  = ImVec4(0.330f, 0.520f, 0.640f, 0.30f);

		// ============================================================================
		// Scrollbar
		// ============================================================================

		colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.025f, 0.027f, 0.032f, 0.70f);
		colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.115f, 0.125f, 0.142f, 0.95f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.165f, 0.180f, 0.205f, 0.95f);
		colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.330f, 0.520f, 0.640f, 0.70f);

		// ============================================================================
		// Slider / Drag Grab
		// ============================================================================

		colors[ImGuiCol_SliderGrab]       = ImVec4(0.330f, 0.520f, 0.640f, 0.80f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.540f, 0.760f, 0.900f, 0.95f);

		// ============================================================================
		// Separator
		// ============================================================================

		colors[ImGuiCol_Separator]        = border_soft;
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.330f, 0.520f, 0.640f, 0.65f);
		colors[ImGuiCol_SeparatorActive]  = ImVec4(0.540f, 0.760f, 0.900f, 0.85f);

		// ============================================================================
		// Tab
		// ============================================================================

		colors[ImGuiCol_Tab]                       = ImVec4(0.045f, 0.048f, 0.055f, 1.00f);
		colors[ImGuiCol_TabHovered]                = ImVec4(0.100f, 0.115f, 0.135f, 1.00f);
		colors[ImGuiCol_TabSelected]               = ImVec4(0.070f, 0.078f, 0.090f, 1.00f);
		colors[ImGuiCol_TabSelectedOverline]       = accent;
		colors[ImGuiCol_TabDimmed]                 = ImVec4(0.035f, 0.037f, 0.042f, 1.00f);
		colors[ImGuiCol_TabDimmedSelected]         = ImVec4(0.055f, 0.060f, 0.070f, 1.00f);
		colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.330f, 0.520f, 0.640f, 0.45f);

		// ============================================================================
		// Table
		// ============================================================================

		colors[ImGuiCol_TableHeaderBg]     = ImVec4(0.065f, 0.072f, 0.084f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.180f, 0.195f, 0.220f, 0.85f);
		colors[ImGuiCol_TableBorderLight]  = ImVec4(0.120f, 0.132f, 0.150f, 0.65f);
		colors[ImGuiCol_TableRowBg]        = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt]     = ImVec4(0.090f, 0.095f, 0.105f, 0.30f);

		// ============================================================================
		// Plot
		// ============================================================================

		colors[ImGuiCol_PlotLines]            = ImVec4(0.420f, 0.620f, 0.740f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]     = ImVec4(0.540f, 0.760f, 0.900f, 1.00f);
		colors[ImGuiCol_PlotHistogram]        = ImVec4(0.460f, 0.560f, 0.700f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.620f, 0.720f, 0.840f, 1.00f);

		// ============================================================================
		// Tree
		// ============================================================================

		colors[ImGuiCol_TreeLines] = ImVec4(0.330f, 0.520f, 0.640f, 0.35f);

		// ============================================================================
		// Docking
		// ============================================================================

		colors[ImGuiCol_DockingPreview] = ImVec4(0.330f, 0.520f, 0.640f, 0.35f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.025f, 0.027f, 0.032f, 1.00f);

		// ============================================================================
		// Navigation / Modal / Overlay
		// ============================================================================

		colors[ImGuiCol_NavCursor]             = accent_hot;
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.540f, 0.760f, 0.900f, 0.65f);
		colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.000f, 0.000f, 0.000f, 0.55f);
		colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.000f, 0.000f, 0.000f, 0.62f);

		// ============================================================================
		// Drag & Drop
		// ============================================================================

		colors[ImGuiCol_DragDropTarget] = ImVec4(0.540f, 0.760f, 0.900f, 0.85f);
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

		initialize_style(&ImGui::GetStyle());

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

				draw->AddRectFilled(min, max, col_u32(panel_color(), alpha * 0.96f), imgui_window_rounding());
				draw->AddRect(min, max, col_u32(imgui_color(ImGuiCol_Border), alpha), imgui_window_rounding());
				draw->AddRectFilled(min, ImVec2(min.x + 4.0f, max.y), col_u32(accent, alpha), imgui_window_rounding(),
				                    ImDrawFlags_RoundCornersLeft);
				float tx = min.x + 16.0f;
				float ty = min.y + 12.0f;
				if (!it->title.empty())
				{
					draw->AddText(ImVec2(tx, ty), col_u32(imgui_color(ImGuiCol_Text), alpha), it->title.c_str());
					ty += ImGui::GetTextLineHeight() + 5.0f;
				}
				draw->AddText(ImVec2(tx, ty), col_u32(text_muted_color(), alpha), it->message.c_str());
				if (!it->action_label.empty())
				{
					const ImVec2 button_size(ImGui::CalcTextSize(it->action_label.c_str()).x + 18.0f, 22.0f);
					const ImVec2 button_min(max.x - button_size.x - 12.0f, max.y - button_size.y - 10.0f);
					const ImVec2 button_max = add(button_min, button_size);
					const bool hovered      = ImGui::IsMouseHoveringRect(button_min, button_max);
					draw->AddRectFilled(button_min, button_max,
					                    col_u32(hovered ? active_context()->style.colors.accent_hovered : accent_color(), alpha),
					                    5.0f);
					draw->AddText(ImVec2(button_min.x + 9.0f, button_min.y + 3.0f), col_u32(imgui_color(ImGuiCol_Text), alpha),
					              it->action_label.c_str());

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
				const float base_rounding = options.rounding >= 0.0f ? options.rounding : imgui_window_rounding();
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
