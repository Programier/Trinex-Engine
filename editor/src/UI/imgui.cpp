#include <Core/base_engine.hpp>
#include <Core/editor_resources.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/etl/templates.hpp>
#include <Core/event.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/keyboard.hpp>
#include <Core/logger.hpp>
#include <Core/math/math.hpp>
#include <Core/mouse.hpp>
#include <Core/package.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_cache.hpp>
#include <Graphics/texture.hpp>
#include <Platform/platform.hpp>
#include <RHI/context.hpp>
#include <RHI/initializers.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <Systems/event_system.hpp>
#include <UI/imgui.hpp>
#include <UI/theme.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include <UI/backend.hpp>

namespace Trinex
{
	//////////////////////////// IMGUI WINDOW BACKEND IMPLEMENTATION ////////////////////////////

	ImGuiWidget::ImGuiWidget() {}

	ImGuiWidgetsList::Node* ImGuiWidgetsList::close_window_internal(Node* node)
	{
		node->widget->on_close();
		return destroy(node);
	}

	ImGuiWidgetsList& ImGuiWidgetsList::close_widget(class ImGuiWidget* widget)
	{
		Node* node = m_root;

		while (node && node->widget != widget)
		{
			node = node->next;
		}

		if (node)
		{
			close_window_internal(node);
		}
		return *this;
	}

	ImGuiWidgetsList& ImGuiWidgetsList::close_all_widgets()
	{
		Node* node = m_root;
		while (node)
		{
			node = close_window_internal(node);
		}
		m_root = nullptr;
		return *this;
	}

	ImGuiWidgetsList& ImGuiWidgetsList::render(class RenderViewport* viewport)
	{
		Node* node = m_root;

		while (node)
		{
			bool status = node->widget->render(viewport);
			node->widget->frame_number += 1;
			if (status)
			{
				node = node->next;
			}
			else
			{
				node = close_window_internal(node);
			}
		}

		return *this;
	}

	ImGuiWidgetsList::Node* ImGuiWidgetsList::destroy(Node* node)
	{
		if (node == m_root)
		{
			m_root = m_root->next;
		}

		if (node->parent)
		{
			node->parent->next = node->next;
		}

		if (node->next)
		{
			node->next->parent = node->parent;
		}

		trx_delete node->widget;

		Node* next = node->next;
		trx_delete node;

		return next;
	}

	ImGuiWidgetsList& ImGuiWidgetsList::push(ImGuiWidget* widget, const void* id)
	{
		Node* parent_node = m_root;
		while (parent_node && parent_node->next) parent_node = parent_node->next;

		Node* node   = trx_new Node();
		node->widget = widget;
		node->parent = parent_node;
		node->next   = nullptr;
		node->id     = id;

		if (parent_node)
		{
			parent_node->next = node;
		}

		if (m_root == nullptr)
		{
			m_root = node;
		}

		return *this;
	}

	ImGuiWidgetsList::~ImGuiWidgetsList()
	{
		while (m_root)
		{
			destroy(m_root);
		}
	}

	static ImGuiWindow* m_current_window = nullptr;

	ImGuiWindow::ImGuiWindow(Trinex::Window* window, ImGuiContext* context)
	{
		m_window  = window;
		m_context = context;

		ImGuiContextLock lock(context);

		UI::Backend::imgui_init(window, context);
		UI::initialize_theme(context);
	}

	ImGuiWindow::~ImGuiWindow()
	{
		ImGuiWindow* current_window = ImGuiWindow::current();

		if (this == current_window)
			current_window = nullptr;

		ImGuiContextLock lock(m_context);
		UI::Backend::imgui_shutdown(nullptr, m_context);

		m_window  = nullptr;
		m_context = nullptr;

		ImGuiWindow::make_current(current_window);
	}

	ImGuiContext* ImGuiWindow::context() const
	{
		return m_context;
	}

	void ImGuiWindow::make_current(ImGuiWindow* window)
	{
		if (m_current_window == window)
			return;

		m_current_window = window;

		if (m_current_window)
		{
			ImGui::SetCurrentContext(window->context());
		}
		else
		{
			ImGui::SetCurrentContext(nullptr);
		}
	}


	ImGuiWindow& ImGuiWindow::new_frame()
	{
		make_current(this);
		UI::Backend::imgui_new_frame(m_window);
		ImGui::NewFrame();
		++m_frame;
		return *this;
	}

	ImGuiWindow& ImGuiWindow::end_frame()
	{
		make_current(this);

		widgets.render(m_window->render_viewport());
		ImGui::Render();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
		}

		{
			auto viewport           = window()->render_viewport();
			RHISwapchain* swapchain = viewport->swapchain();

			RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
			{
				auto texture = swapchain->as_texture();
				auto rtv     = texture->as_rtv();

				ctx->barrier(texture, RHIAccess::TransferDst);
				ctx->clear_rtv(rtv, 0.f, 0.f, 0.f, 1.f);
				ctx->barrier(texture, RHIAccess::RTV);

				ImGuiContextLock lock(m_context);


				UI::Backend::imgui_render(ctx, window(), ImGui::GetDrawData());

				ctx->barrier(texture, RHIAccess::PresentSrc);
			}
			RHIContextPool::global_instance()->end_context(ctx, swapchain->acquire_semaphore(), swapchain->present_semaphore());
			RHI::instance()->present(swapchain);
		}

		make_current(nullptr);

		return *this;
	}

	Trinex::Window* ImGuiWindow::window() const
	{
		return m_window;
	}

	usize ImGuiWindow::frame_index() const
	{
		return m_frame;
	}

	ImGuiWindow* ImGuiWindow::current()
	{
		return m_current_window;
	}
}// namespace Trinex

namespace ImGui
{
	struct InputTextCallback {
		ImGuiInputTextCallback callback;
		void* userdata;
		Trinex::String* str;
	};

	static int input_text_callback(ImGuiInputTextCallbackData* data)
	{
		InputTextCallback* userdata = reinterpret_cast<InputTextCallback*>(data->UserData);

		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			Trinex::String* str = userdata->str;
			const int new_size  = data->BufTextLen;
			str->resize(new_size);
			data->Buf = str->data();
		}

		if (userdata->callback)
		{
			data->UserData = userdata->userdata;
			return userdata->callback(data);
		}

		return 0;
	}

	void Paint(ImVec2 size, ImDrawCallback callback, void* userdata, size_t userdata_size)
	{
		ImGuiWindow* window = GetCurrentWindow();

		if (!window)
			return;

		Dummy(size);

		if (!IsItemVisible())
			return;

		ImDrawList* list  = window->DrawList;
		ImGuiViewport* vp = window->Viewport;

		struct Args {
			Trinex::Vector2f16 pos;
			Trinex::Vector2f16 size;
		} args;

		args.pos  = EngineVecFrom((ImGui::GetItemRectMin() - vp->Pos) / vp->Size);
		args.size = EngineVecFrom(size / vp->Size);

		ImDrawCallback viewport_setup = [](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
			Args* args = reinterpret_cast<Args*>(cmd->UserCallbackData);

			auto ctx = Trinex::UI::Backend::rhi();
			ctx->viewport(Trinex::RHIViewport(args->size, args->pos));
			ctx->scissor(Trinex::RHIScissor(args->size, args->pos));
		};

		list->AddCallback(viewport_setup, &args, sizeof(args));
		list->AddCallback(callback, userdata, userdata_size);
		list->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}

	void TextEllipsis(const char* text, float max_width)
	{
		if (!text || max_width <= 0.0f)
			return;

		const char* end_ptr   = text + strlen(text);
		float text_full_width = ImGui::CalcTextSize(text, end_ptr).x;

		if (text_full_width <= max_width)
		{
			ImGui::TextUnformatted(text);
			return;
		}

		const char* ellipsis = "...";
		float ellipsis_width = ImGui::CalcTextSize(ellipsis).x;
		const char* cut_ptr  = text;

		while (cut_ptr < end_ptr)
		{
			float current_width = ImGui::CalcTextSize(text, cut_ptr + 1).x;
			if (current_width + ellipsis_width > max_width)
				break;
			++cut_ptr;
		}

		ImGui::TextUnformatted(text, cut_ptr);
		ImGui::SameLine(0, 0);
		ImGui::TextUnformatted(ellipsis);
	}

	bool InputText(const char* label, Trinex::String& buffer, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback,
	               void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputText(label, buffer.data(), buffer.size() + 1, flags, input_text_callback, &data);
	}

	bool InputTextMultiline(const char* label, Trinex::String& buffer, const ImVec2& size, ImGuiInputTextFlags flags,
	                        ImGuiInputTextCallback callback, void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputTextMultiline(label, buffer.data(), buffer.size() + 1, size, flags, input_text_callback, &data);
	}

	bool InputTextWithHint(const char* label, const char* hint, Trinex::String& buffer, ImGuiInputTextFlags flags,
	                       ImGuiInputTextCallback callback, void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputTextWithHint(label, hint, buffer.data(), buffer.size() + 1, flags, input_text_callback, &data);
	}

	bool ImageButton(ImTextureID user_texture_id, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1,
	                 const ImVec4& bg_col, const ImVec4& tint_col)
	{
		ImGuiContext& g     = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		return ImageButtonEx(window->GetID(static_cast<const void*>(user_texture_id)), user_texture_id, image_size, uv0, uv1,
		                     bg_col, tint_col);
	}

	float TableGetAutoWidth(const char* name)
	{
		return TableGetAutoWidth(ImGui::GetID(name));
	}

	float TableGetAutoWidth(ImGuiID table_id)
	{
		ImGuiContext& g   = *GImGui;
		ImGuiTable* table = g.Tables.GetByKey(table_id);

		if (table == nullptr)
			return 0.f;

		const ImGuiStyle& style = g.Style;
		const int columnCount   = table->ColumnsCount;

		float width = 0.f;

		if (columnCount > 0)
		{
			width = table->Columns[columnCount - 1].WidthAuto;

			for (int i = 0, end = columnCount - 1; i < end; ++i)
			{
				width += table->Columns[i].WidthGiven;
			}
		}

		width += static_cast<float>(columnCount) * style.CellPadding.x * 2.0f;

		float spacing = (table->Flags & ImGuiTableFlags_BordersInnerV) ? table->CellSpacingX2 : table->CellSpacingX1;
		if (columnCount > 1)
			width += columnCount * spacing;

		width += table->OuterPaddingX * 2.0f;

		if (table->Flags & ImGuiTableFlags_BordersInnerV)
			width += (columnCount - 1) * style.FrameBorderSize;

		if (table->Flags & ImGuiTableFlags_BordersOuterV)
			width += 2.0f * style.FrameBorderSize;

		return width;
	}

	bool InputScalarWithPrefix(const char* prefix, ImGuiDataType data_type, void* p_data, const void* p_step,
	                           const void* p_step_fast, const char* format, ImGuiInputTextFlags flags)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		const float prefix_w = ImGui::CalcTextSize(prefix).x + style.FramePadding.x * 2.0f;

		ImGui::PushID(prefix);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x + prefix_w, style.FramePadding.y));

		bool changed = ImGui::InputScalar("##input", data_type, p_data, p_step, p_step_fast, format, flags);

		ImGui::PopStyleVar();

		ImDrawList* dl = ImGui::GetWindowDrawList();
		auto min       = ImGui::GetItemRectMin();

		const ImVec2 text_pos(min.x + style.FramePadding.x, min.y + (ImGui::GetItemRectSize().y - ImGui::GetFontSize()) * 0.5f);

		dl->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), prefix);

		ImGui::PopID();
		return changed;
	}

	bool InputScalarWithPrefix(const char* prefix, ImU32 prefix_bg, ImGuiDataType data_type, void* p_data, const void* p_step,
	                           const void* p_step_fast, const char* format, ImGuiInputTextFlags flags)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		const float prefix_w = ImGui::CalcTextSize(prefix).x + style.FramePadding.x * 2.0f;

		ImGui::PushID(prefix);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x + prefix_w, style.FramePadding.y));

		bool changed = ImGui::InputScalar("##input", data_type, p_data, p_step, p_step_fast, format, flags);

		ImGui::PopStyleVar();

		ImDrawList* dl = ImGui::GetWindowDrawList();

		ImRect rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

		ImRect prefix_rect(rect.Min, ImVec2(rect.Min.x + prefix_w, rect.Max.y));
		dl->AddRectFilled(prefix_rect.Min, prefix_rect.Max, prefix_bg, style.FrameRounding, ImDrawFlags_RoundCornersLeft);

		const ImVec2 text_pos(rect.Min.x + style.FramePadding.x,
		                      rect.Min.y + (ImGui::GetItemRectSize().y - ImGui::GetFontSize()) * 0.5f);

		dl->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), prefix);

		ImGui::PopID();
		return changed;
	}

	void PushIconFont()
	{
		ImGui::PushFont(Trinex::UI::icons_font());
	}

	bool IconButton(const char* icon, float size, ImGuiButtonFlags flags)
	{
		PushIconFont();
		bool status = ImGui::ButtonEx(icon, {size, size}, flags);
		ImGui::PopFont();
		return status;
	}
}// namespace ImGui
