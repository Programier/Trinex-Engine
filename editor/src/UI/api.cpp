#include "api_internal.hpp"

namespace Trinex::UI
{
	Context* g_context = nullptr;

	/////////////////////// LIFECYCLE AND FRAME ///////////////////////

	void initialize()
	{
		active_context()->style = Style{};
	}

	void shutdown()
	{
		active_context()->style_stack.clear();
		g_anim.clear();
		g_open.clear();
		g_hover_time.clear();
		g_notification_y.clear();
		g_active_combo           = 0;
		g_active_combo_field_min = ImVec2(0.0f, 0.0f);
		g_active_combo_field_max = ImVec2(0.0f, 0.0f);
		g_active_combo_popup_min = ImVec2(0.0f, 0.0f);
		g_active_combo_popup_max = ImVec2(0.0f, 0.0f);
		g_menu_bar_style_depth   = 0;
		g_menu_popup_style_depth = 0;
		g_menu_alpha_depth       = 0;
		g_table_style_depth      = 0;
		g_tree_indent_stack.clear();
		g_tree_stack.clear();
		g_area_stack.clear();
		g_panel_stack.clear();
		g_glass_panel_stack.clear();
		g_card_stack.clear();
		g_shadow_stack.clear();
		g_disabled_alpha_stack.clear();
		g_pending_modals.clear();
		g_pending_popups.clear();
		g_command_palette.commands.clear();
		g_command_palette.filtered_indices.clear();
		g_command_palette.open                    = false;
		g_command_palette.focus_search_next_frame = false;
		g_command_palette.search[0]               = '\0';
		g_command_palette.selected_index          = 0;
		g_notifications.clear();
		active_context()->keybind_capture = 0;
		active_context()->draw_alpha      = 1.0f;

		destroy_list(active_context()->window_list);
	}

	Context* create_context(Trinex::Window* window)
	{
		Context* ctx = trx_new Context();
		ctx->window  = window;
		ctx->context = ImGui::CreateContext();

		UI::Backend::imgui_init(window, ctx->context);

		auto& io = ImGui::GetIO();
		initialize_fonts(ctx);

		io.Fonts->Build();
		io.FontDefault = ctx->fonts[font_family_index(FontFamily::Default)][font_size_index(FontSize::Normal)];
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		return ctx;
	}

	void destroy_context(Context* context)
	{
		UI::Backend::imgui_shutdown(context->window, context->context);
		ImGui::DestroyContext(context->context);
		trx_delete context;
	}

	bool begin_frame(Context* context)
	{
		trinex_assert(context);
		g_context = context;

		ImGui::SetCurrentContext(context->context);
		UI::Backend::imgui_new_frame(context->window);
		ImGui::NewFrame();

		return true;
	}

	void end_frame()
	{
		trinex_assert(g_context);

		render_windows();
		render_notifications();
		ImGui::Render();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
		}

		{
			auto viewport           = g_context->window->render_viewport();
			RHISwapchain* swapchain = viewport->swapchain();

			RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
			{
				auto texture = swapchain->as_texture();
				auto rtv     = texture->as_rtv();

				ctx->barrier(texture, RHIAccess::TransferDst);
				ctx->clear_rtv(rtv, 0.f, 0.f, 0.f, 1.f);
				ctx->barrier(texture, RHIAccess::RTV);

				UI::Backend::imgui_render(ctx, g_context->window, ImGui::GetDrawData());

				ctx->barrier(texture, RHIAccess::PresentSrc);
			}

			RHIContextPool::global_instance()->end_context(ctx, swapchain->acquire_semaphore(), swapchain->present_semaphore());
			RHI::instance()->present(swapchain);
		}

		trinex_assert(g_shadow_stack.empty() && "UI::push_shadow()/pop_shadow() imbalance detected at end_frame()");
		if (!g_shadow_stack.empty())
		{
			g_shadow_stack.clear();
		}

		trinex_assert(g_glass_panel_stack.empty() &&
		              "UI::begin_glass_panel()/end_glass_panel() imbalance detected at end_frame()");
		if (!g_glass_panel_stack.empty())
		{
			g_glass_panel_stack.clear();
		}

		active_context()->allocator.reset();
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

	/////////////////////// STYLE AND EFFECTS ///////////////////////

	Style& style()
	{
		return active_context()->style;
	}

	void style(const Style& value)
	{
		active_context()->style = value;
	}

	void push_style(const Style& value)
	{
		active_context()->style_stack.push_back(active_context()->style);
		active_context()->style = value;
	}

	void pop_style()
	{
		if (!active_context()->style_stack.empty())
		{
			active_context()->style = active_context()->style_stack.back();
			active_context()->style_stack.pop_back();
		}
	}

	void push_shadow(const Shadow& shadow)
	{
		g_shadow_stack.push_back(shadow);
	}

	void pop_shadow()
	{
		trinex_assert(!g_shadow_stack.empty() && "UI::pop_shadow() called without matching push_shadow()");
		if (!g_shadow_stack.empty())
		{
			g_shadow_stack.pop_back();
		}
	}

	void push_blur(const BlurOptions& options)
	{
		g_blur_stack.push_back(options);
	}

	void pop_blur()
	{
		trinex_assert(!g_blur_stack.empty() && "UI::pop_blur() called without matching push_blur()");
		if (!g_blur_stack.empty())
		{
			g_blur_stack.pop_back();
		}
	}

	void blur(const Vec2& min, const Vec2& max, DrawList draw_list, const BlurOptions& options)
	{
		const float spread = std::max(0.0f, options.spread);
		const Vec2 pad(spread, spread);
		const Vec2 area_min  = min - pad;
		const Vec2 area_max  = max + pad;
		const Vec2 area_size = area_max - area_min;

		if (options.radius > 0.0f)
		{
			paint(draw_list, [options, area_min, area_max]() {
				const float radius = Math::clamp(options.radius, 0.0f, 64.0f);

				if (radius <= 0.0f)
				{
					return;
				}

				const float sigma           = options.sigma > 0.0f ? options.sigma : std::max(1.0f, radius * 0.45f);
				const RHITextureFlags flags = RHITextureFlags::ColorAttachment;
				RHITexturePool* pool        = RHITexturePool::global_instance();

				RHIContext* ctx              = Backend::rhi();
				RHITexture* window           = Backend::render_target();
				const Vector2u viewport_size = window->size();
				RHITexture* temporary        = pool->request_surface(RHISurfaceFormat::RGBA8, viewport_size, flags);

				const Vector2f blur_offset = area_min / Vector2f(viewport_size);
				const Vector2f blur_size   = (area_max - area_min) / Vector2f(viewport_size);

				ctx->push_debug_stage("Bloor");

				ctx->end_rendering();
				ctx->barrier(window, RHIAccess::SRVGraphics);
				ctx->barrier(temporary, RHIAccess::RTV);

				ctx->begin_rendering(temporary->as_rtv());
				Pipelines::GaussianBlur::blur(ctx, window->as_srv(), {0.f, 1.f / static_cast<f32>(viewport_size.y)}, sigma,
				                              radius, {}, nullptr, blur_offset, blur_size);
				ctx->end_rendering();

				ctx->barrier(window, RHIAccess::RTV);
				ctx->barrier(temporary, RHIAccess::SRVGraphics);
				ctx->begin_rendering(window->as_rtv());
				Pipelines::GaussianBlur::blur(ctx, temporary->as_srv(), {1.f / static_cast<f32>(viewport_size.x), 0.f}, sigma,
				                              radius, {}, nullptr, blur_offset, blur_size);

				if (options.noise_opacity > 0.f)
				{
					ctx->push_debug_stage("Noise");
					Pipelines::NoiseApplication::noise(ctx, options.noise_opacity, options.noise_scale, blur_offset, blur_size);
					ctx->pop_debug_stage();
				}

				ctx->pop_debug_stage();


				pool->return_surface(temporary);
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
				                    std::max(0.0f, base_rounding + spread));
			}
		}
	}

	void paint(Vec2 pos, Vec2 size, PaintFunction function, void* userdata, usize userdata_size, DrawList draw_list)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		if (function == nullptr)
			return;

		ImGuiViewport* viewport = nullptr;
		ImDrawList* list        = resolve_draw_list(draw_list, window, viewport);
		add_paint_callback(list, viewport, pos, size, function, userdata, userdata_size);
	}

	void paint(Vec2 size, PaintFunction function, void* userdata, usize userdata_size, DrawList draw_list)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiViewport* vp   = window->Viewport;

		Vec2 pos = to_vec(ImGui::GetItemRectMin());
		paint(pos, size, function, userdata, userdata_size, draw_list);
	}

	void paint(PaintFunction function, void* userdata, usize userdata_size, DrawList draw_list)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiViewport* vp   = window ? window->Viewport : ImGui::GetMainViewport();

		Vec2 pos  = to_vec(vp->Pos);
		Vec2 size = to_vec(vp->Size);
		paint(pos, size, function, userdata, userdata_size, draw_list);
	}

	/////////////////////// ANIMATION AND IDENTITY ///////////////////////

	float apply_ease(float t, Ease mode)
	{
		t = Math::clamp(t, 0.f, 1.f);
		switch (mode)
		{
			case Ease::Linear: return t;
			case Ease::InQuad: return t * t;
			case Ease::OutQuad: return 1.0f - (1.0f - t) * (1.0f - t);
			case Ease::InOutQuad: return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
			case Ease::OutBack:
			{
				const float c1 = 1.70158f;
				const float c3 = c1 + 1.0f;
				return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
			}
			case Ease::OutCubic:
			default: return 1.0f - std::pow(1.0f - t, 3.0f);
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
		g_anim.erase(to_imgui_id(id));
	}

	void clear_animations()
	{
		g_anim.clear();
	}

	void push_id(const char* id)
	{
		ImGui::PushID(id);
	}

	void push_id(int id)
	{
		ImGui::PushID(id);
	}

	void pop_id()
	{
		ImGui::PopID();
	}

	ID id(const char* id)
	{
		return to_ui_id(ImGui::GetID(id));
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

	static void dock_builder_begin(ID dockspace_id, const Vec2& size, DockNodeFlags flags)
	{
		const ImGuiID root_id      = to_imgui_id(dockspace_id);
		ImGuiViewport* viewport    = ImGui::GetMainViewport();
		const ImVec2 fallback_size = viewport != nullptr ? viewport->WorkSize : ImVec2(0.0f, 0.0f);
		const ImVec2 resolved_size(size.x > 0.0f ? size.x : fallback_size.x, size.y > 0.0f ? size.y : fallback_size.y);

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

	static void dock_builder_dock_window(const char* window_name, ID dock_id)
	{
		ImGui::DockBuilderDockWindow(window_name, to_imgui_id(dock_id));
	}

	bool DockLayoutBuilder::exists() const
	{
		return ImGui::DockBuilderGetNode(to_imgui_id(m_root)) != nullptr;
	}

	DockLayoutBuilder& DockLayoutBuilder::bind(const char* id, DockID dock)
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

	DockID DockLayoutBuilder::find(const char* id) const
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

	DockID DockLayoutBuilder::require(const char* id) const
	{
		const DockID dock = find(id);
		trinex_assert(dock && "UI::DockLayoutBuilder::require() cannot find named dock");
		return dock;
	}

	bool DockLayoutBuilder::has(const char* id) const
	{
		return static_cast<bool>(find(id));
	}

	DockLayoutBuilder& DockLayoutBuilder::flags(DockID dock_id, DockNodeFlags flags)
	{
		dock_builder_set_flags(dock_id, flags);
		return *this;
	}

	DockLayoutBuilder& DockLayoutBuilder::flags(const char* id, DockNodeFlags flags)
	{
		return this->flags(require(id), flags);
	}

	DockBuilderSplitResult DockLayoutBuilder::split(DockID dock, DockSplitDir dir, float ratio, const char* id)
	{
		DockBuilderSplitResult result = dock_builder_split(dock, dir, ratio);
		bind(id, result.child);
		return result;
	}

	DockBuilderSplitResult DockLayoutBuilder::split(DockID dock, DockSplitDir dir, float ratio, const char* remainder_id,
	                                                const char* child_id)
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

	DockID DockLayoutBuilder::crop(DockID& dock, DockSplitDir dir, float ratio, const char* id)
	{
		auto result = split(dock, dir, ratio, id);
		dock        = result.remainder;
		return result.child;
	}

	DockID DockLayoutBuilder::crop(DockID& dock, DockSplitDir dir, float ratio, const char* remainder_id, const char* child_id)
	{
		auto result = split(dock, dir, ratio, remainder_id, child_id);
		dock        = result.remainder;
		return result.child;
	}

	DockID DockLayoutBuilder::dock(const char* window_name, DockID dock_id)
	{
		dock_builder_dock_window(window_name, dock_id);
		return dock_id;
	}

	DockID DockLayoutBuilder::dock(const char* window_name, const char* dock_id)
	{
		return dock(window_name, require(dock_id));
	}

	bool DockLayoutBuilder::begin(Vec2 size, DockNodeFlags flags)
	{
		const ImGuiID root = ImGui::GetID("##dockspace");

		m_root = ID(root);
		m_main = ID(root);
		m_named.clear();

		const ImVec2 fallback_size = ImGui::GetWindowSize();
		const ImVec2 resolved_size(size.x > 0.0f ? size.x : fallback_size.x, size.y > 0.0f ? size.y : fallback_size.y);

		ImGui::DockBuilderRemoveNode(root);
		ImGui::DockBuilderAddNode(root, to_imgui_dock_node_flags(flags) | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(root, resolved_size);
		return true;
	}

	bool DockLayoutBuilder::begin(DockID root_id, Vec2 size, DockNodeFlags flags)
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
		const ImVec2 resolved_size(size.x > 0.0f ? size.x : fallback_size.x, size.y > 0.0f ? size.y : fallback_size.y);

		ImGui::DockBuilderRemoveNode(root);
		ImGui::DockBuilderAddNode(root, to_imgui_dock_node_flags(flags) | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(root, resolved_size);
		return true;
	}

	DockLayoutBuilder& DockLayoutBuilder::end()
	{
		ImGui::DockBuilderFinish(to_imgui_id(m_root));
		return *this;
	}

	bool begin_dockspace(const DockLayoutOptions& options)
	{
		ImGuiID id = options.id ? to_imgui_id(options.id) : ImGui::GetID("##dockspace");

		if (!(id = ImGui::DockSpace(id, to_imvec(options.size), to_imgui_dock_node_flags(options.flags))))
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

	void dockspace(const DockLayoutOptions& options, const FunctionRef<void(DockLayoutBuilder&)>& function)
	{
		if (begin_dockspace(options))
		{
			DockLayoutBuilder builder;
			if (builder.begin(options.id, options.size, options.flags))
			{
				function(builder);
				builder.end();
			}
			end_docspace();
		}
	}

	void dockspace(const FunctionRef<void(DockLayoutBuilder&)>& builder)
	{
		dockspace({}, builder);
	}

	void viewport_dockspace(const DockLayoutOptions& options, const FunctionRef<void(DockLayoutBuilder&)>& function)
	{
		if (begin_viewport_dockspace(options))
		{
			DockLayoutBuilder builder;
			if (builder.begin(options.id, options.size, options.flags))
			{
				function(builder);
				builder.end();
			}
			end_viewport_dockspace();
		}
	}

	void viewport_dockspace(const FunctionRef<void(DockLayoutBuilder&)>& builder)
	{
		viewport_dockspace({}, builder);
	}

}// namespace Trinex::UI
