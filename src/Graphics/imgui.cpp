#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/render_resource.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <Window/window.hpp>
#include <imgui.h>

namespace Engine::ImGuiBackend
{
	extern void imgui_trinex_rhi_render_draw_data(ImGuiContext* ctx, ImDrawData* draw_data);
}

namespace Engine::ImGuiRenderer
{
	ENGINE_EXPORT float rhi_rendering_scale_factor = 1.f;

	ENGINE_EXPORT ImTextureID create_fonts_texture(const byte* fonts_texture_data, float fonts_texture_x, float fonts_texture_y)
	{
		ImTextureID texture = {};

		texture.texture = Object::new_instance<EngineResource<Texture2D>>(
		        Strings::format("FontsTexture {}", reinterpret_cast<size_t>(ImGui::GetCurrentContext())));

		texture.texture->flags(Object::IsAvailableForGC, false);
		texture.texture->init(ColorFormat::R8G8B8A8, Size2D(fonts_texture_x, fonts_texture_y), fonts_texture_data,
		                      static_cast<size_t>(fonts_texture_x * fonts_texture_y * 4.f));
		auto package = Package::static_find_package("Engine::ImGui", true);
		package->add_object(texture.texture);

		texture.sampler = Object::new_instance<EngineResource<Sampler>>(
		        Strings::format("Sampler {}", reinterpret_cast<size_t>(ImGui::GetCurrentContext())));
		texture.sampler->filter = SamplerFilter::Trilinear;
		texture.sampler->rhi_create();
		texture.sampler->flags(Object::IsAvailableForGC, false);
		package->add_object(texture.sampler);
		return texture;
	}

	ImDrawData* DrawData::draw_data()
	{
		return &m_draw_data[m_render_index];
	}

	static void release_draw_data(ImDrawData& data)
	{
		if (data.CmdListsCount > 0)
		{
			for (int index = 0; index < data.CmdListsCount; index++)
			{
				ImDrawList* drawList = data.CmdLists[index];
				IM_DELETE(drawList);
			}

			data.CmdLists.clear();
		}

		data.Clear();
	}

	DrawData& DrawData::release(bool full)
	{

		release_draw_data(m_draw_data[m_logic_index]);
		if (full)
			release_draw_data(m_draw_data[m_render_index]);

		return *this;
	}

	DrawData& DrawData::copy(ImDrawData* draw_data)
	{
		release(false);

		m_draw_data[m_logic_index] = *draw_data;

		m_draw_data[m_logic_index].CmdLists.resize(draw_data->CmdListsCount);
		for (int index = 0; index < draw_data->CmdListsCount; index++)
		{
			ImDrawList* drawList                       = draw_data->CmdLists[index]->CloneOutput();
			m_draw_data[m_logic_index].CmdLists[index] = drawList;
		}

		return *this;
	}

	DrawData& DrawData::swap_render_index()
	{
		m_render_index = (m_render_index + 1) % 2;
		return *this;
	}

	DrawData& DrawData::swap_logic_index()
	{
		m_logic_index = (m_logic_index + 1) % 2;
		return *this;
	}

	DrawData::~DrawData()
	{
		release(true);
	}

	ImGuiAdditionalWindow::ImGuiAdditionalWindow()
	{}

	ImGuiAdditionalWindowList::Node* ImGuiAdditionalWindowList::close_window_internal(Node* node)
	{
		node->window->on_close();
		return destroy(node);
	}

	ImGuiAdditionalWindowList& ImGuiAdditionalWindowList::close_window(class ImGuiAdditionalWindow* window)
	{
		Node* node = m_root;

		while (node && node->window != window)
		{
			node = node->next;
		}

		if (node)
		{
			close_window_internal(node);
		}
		return *this;
	}

	ImGuiAdditionalWindowList& ImGuiAdditionalWindowList::close_all_windows()
	{
		Node* node = m_root;
		while (node)
		{
			node = close_window_internal(node);
		}
		m_root = nullptr;
		return *this;
	}

	ImGuiAdditionalWindowList& ImGuiAdditionalWindowList::render(class RenderViewport* viewport)
	{
		Node* node = m_root;

		while (node)
		{
			bool status = node->window->render(viewport);
			node->window->frame_number += 1;
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

	void ImGuiAdditionalWindow::init(class RenderViewport* viewport)
	{}

	ImGuiAdditionalWindowList::Node* ImGuiAdditionalWindowList::destroy(Node* node)
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

		delete node->window;

		Node* next = node->next;
		delete node;

		return next;
	}

	ImGuiAdditionalWindowList& ImGuiAdditionalWindowList::push(ImGuiAdditionalWindow* window, const void* id)
	{
		window->init(Window::current()->window()->render_viewport());
		Node* parent_node = m_root;
		while (parent_node && parent_node->next) parent_node = parent_node->next;

		Node* node   = new Node();
		node->window = window;
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

	ImGuiAdditionalWindowList::~ImGuiAdditionalWindowList()
	{
		while (m_root)
		{
			destroy(m_root);
		}
	}

	implement_class_default_init(Engine::ImGuiRenderer, Window, 0);

	static Window* m_current_window = nullptr;

	Window::Window(Engine::Window* window, ImGuiContext* ctx) : m_frame(0), m_context(ctx), m_window(window)
	{}

	Window& Window::free_resources()
	{
		on_destroy();
		return *this;
	}

	ImGuiContext* Window::context() const
	{
		return m_context;
	}

	ImDrawData* Window::draw_data()
	{
		return m_draw_data.draw_data();
	}

	void Window::make_current(Window* window)
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


	Window& Window::new_frame()
	{
		make_current(this);
		m_window->imgui_new_frame();
		ImGui::NewFrame();

		++m_frame;
		return *this;
	}

	Window& Window::end_frame()
	{
		make_current(this);

		window_list.render(m_window->render_viewport());
		ImGui::Render();

		m_draw_data.copy(ImGui::GetDrawData());
		m_draw_data.swap_logic_index();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
		}

		make_current(nullptr);

		return *this;
	}

	Window& Window::rhi_render()
	{
		ImGuiBackend::imgui_trinex_rhi_render_draw_data(m_context, draw_data());
		m_draw_data.swap_render_index();
		return *this;
	}

	Engine::Window* Window::window() const
	{
		return m_window;
	}

	size_t Window::frame_index() const
	{
		return m_frame;
	}

	Window& Window::reset_frame_index()
	{
		m_frame = 0;
		return *this;
	}

	Window* Window::current()
	{
		return m_current_window;
	}

	Window::~Window()
	{
		if (m_window)
		{
			m_window->imgui_terminate();
		}
	}

	struct InputTextCallback {
		ImGuiInputTextCallback callback;
		void* userdata;
		String* str;
	};

	static int input_text_callback(ImGuiInputTextCallbackData* data)
	{
		InputTextCallback* userdata = reinterpret_cast<InputTextCallback*>(data->UserData);

		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			String* str        = userdata->str;
			const int new_size = data->BufTextLen;
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

	bool ENGINE_EXPORT IsMouseDownNow(ImGuiMouseButton button)
	{
		return ImGui::GetIO().MouseDownDuration[button] == 0.f && ImGui::IsMouseDown(button);
	}

	bool ENGINE_EXPORT InputText(const char* label, String& buffer, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback,
	                             void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputText(label, buffer.data(), buffer.size() + 1, flags, input_text_callback, &data);
	}

	bool ENGINE_EXPORT InputTextMultiline(const char* label, String& buffer, const ImVec2& size, ImGuiInputTextFlags flags,
	                                      ImGuiInputTextCallback callback, void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputTextMultiline(label, buffer.data(), buffer.size() + 1, size, flags, input_text_callback, &data);
	}

	bool ENGINE_EXPORT InputTextWithHint(const char* label, const char* hint, String& buffer, ImGuiInputTextFlags flags,
	                                     ImGuiInputTextCallback callback, void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputTextWithHint(label, hint, buffer.data(), buffer.size() + 1, flags, input_text_callback, &data);
	}

	bool ENGINE_EXPORT BeginPopup(const char* name, ImGuiWindowFlags flags, bool (*callback)(void*), void* userdata)
	{
		bool status = ImGui::BeginPopup(name, flags);

		if (status)
		{
			if (callback)
			{
				status = callback(userdata);
				if (status == false)
				{
					ImGui::CloseCurrentPopup();
				}
			}

			bool hovered = IsWindowRectHovered();

			if (!ImGui::IsWindowAppearing() && !hovered)
			{
				for (int_t i = 0; i < ImGuiMouseButton_COUNT; ++i)
				{
					if (IsMouseDownNow(i))
					{
						ImGui::CloseCurrentPopup();
						status = false;
						break;
					}
				}
			}

			ImGui::EndPopup();
		}

		return status;
	}

	bool ENGINE_EXPORT IsWindowRectHovered()
	{
		auto pos     = ImGui::GetWindowPos();
		auto end_pos = pos + ImGui::GetWindowSize();

		return ImGui::IsMouseHoveringRect(pos, end_pos);
	}
}// namespace Engine::ImGuiRenderer
