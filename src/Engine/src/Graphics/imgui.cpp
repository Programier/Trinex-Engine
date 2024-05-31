#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/render_resource.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <Window/window_interface.hpp>
#include <imgui.h>

namespace Engine::ImGuiRenderer
{
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

    ViewportClient& ImGuiViewportClient::on_bind_viewport(class RenderViewport* viewport)
    {
        m_window = Window::current();
        return *this;
    }

    ViewportClient& ImGuiViewportClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        rhi->imgui_render(m_window->context(), m_draw_data.draw_data());
        m_draw_data.swap_render_index();
        return *this;
    }

    ViewportClient& ImGuiViewportClient::update(class RenderViewport*, float dt)
    {
        m_draw_data.copy(viewport->DrawData);
        m_draw_data.swap_logic_index();
        return *this;
    }

    implement_engine_class_default_init(ImGuiViewportClient);

    static Window* m_current_window = nullptr;

    Window::Window(Engine::Window* window, ImGuiContext* ctx) : m_context(ctx), m_window(window)
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
        m_window->interface()->new_imgui_frame();
        rhi->imgui_new_frame(m_context);
        ImGui::NewFrame();
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

    Window& Window::render()
    {
        rhi->imgui_render(m_context, draw_data());
        m_draw_data.swap_render_index();
        return *this;
    }

    Engine::Window* Window::window() const
    {
        return m_window;
    }

    Window* Window::current()
    {
        return m_current_window;
    }

    Window::~Window()
    {}


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
