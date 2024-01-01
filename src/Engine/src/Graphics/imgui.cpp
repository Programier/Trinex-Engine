#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/render_resource.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <Window/window_interface.hpp>
#include <imgui.h>

#include <imfilebrowser.h>

namespace Engine::ImGuiRenderer
{
    ImDrawData* DrawData::draw_data()
    {
        return &_M_draw_data;
    }

    DrawData& DrawData::release()
    {
        if (_M_draw_data.CmdListsCount > 0)
        {
            for (int index = 0; index < _M_draw_data.CmdListsCount; index++)
            {
                ImDrawList* drawList = _M_draw_data.CmdLists[index];
                IM_DELETE(drawList);
            }

            _M_draw_data.CmdLists.clear();
        }

        _M_draw_data.Clear();
        return *this;
    }

    DrawData& DrawData::copy(ImDrawData* draw_data)
    {
        release();

        _M_draw_data = *draw_data;

        _M_draw_data.CmdLists.resize(draw_data->CmdListsCount);
        for (int index = 0; index < draw_data->CmdListsCount; index++)
        {
            ImDrawList* drawList         = draw_data->CmdLists[index]->CloneOutput();
            _M_draw_data.CmdLists[index] = drawList;
        }

        return *this;
    }

    DrawData::~DrawData()
    {
        release();
    }


    ImGuiTexture::ImGuiTexture() : _M_handle(nullptr)
    {}

    struct ImGuiTextureInitTask : public ExecutableObject {
        ImGuiContext* _M_ctx           = nullptr;
        ImGuiTexture* _M_imgui_texture = nullptr;
        Texture* _M_texture            = nullptr;
        Sampler* _M_sampler            = nullptr;

        ImGuiTextureInitTask(ImGuiContext* ctx, ImGuiTexture* imgui_texture, Texture* texture, Sampler* sampler)
            : _M_ctx(ctx), _M_imgui_texture(imgui_texture), _M_texture(texture), _M_sampler(sampler)
        {}

        int_t execute() override
        {
            _M_imgui_texture->init(_M_ctx, _M_texture, _M_sampler);
            return sizeof(ImGuiTextureInitTask);
        }
    };

    ImGuiTexture& ImGuiTexture::init(ImGuiContext* ctx, Texture* texture, Sampler* sampler)
    {
        release();
        Thread* render_thread = engine_instance->thread(ThreadType::RenderThread);
        if (Thread::this_thread() == render_thread)
        {
            _M_handle = engine_instance->rhi()->imgui_create_texture(ctx, texture, sampler);
        }
        else
        {
            render_thread->insert_new_task<ImGuiTextureInitTask>(ctx, this, texture, sampler);
        }
        return *this;
    }

    void* ImGuiTexture::handle() const
    {
        if (!_M_handle)
            return nullptr;
        return _M_handle->handle();
    }


    ImGuiTexture& ImGuiTexture::release()
    {
        if (_M_handle)
            RenderResource::release_render_resouce(_M_handle);
        _M_handle = nullptr;
        return *this;
    }

    ImGuiTexture::~ImGuiTexture()
    {
        release();
    }

    ImGuiAdditionalWindow::ImGuiAdditionalWindow()
    {}

    ImGuiAdditionalWindowList& ImGuiAdditionalWindowList::render(class RenderViewport* viewport)
    {
        Node* node = _M_root;

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
                node = destroy(node);
            }
        }

        return *this;
    }

    void ImGuiAdditionalWindow::init(class RenderViewport* viewport)
    {}

    ImGuiAdditionalWindowList::Node* ImGuiAdditionalWindowList::destroy(Node* node)
    {
        if (node == _M_root)
        {
            _M_root = _M_root->next;
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

    ImGuiAdditionalWindowList& ImGuiAdditionalWindowList::push(class RenderViewport* viewport, ImGuiAdditionalWindow* window)
    {
        window->init(viewport);
        Node* parent_node = _M_root;
        while (parent_node && parent_node->next) parent_node = parent_node->next;

        Node* node   = new Node();
        node->window = window;
        node->parent = parent_node;
        node->next   = nullptr;

        if (parent_node)
        {
            parent_node->next = node;
        }

        if (_M_root == nullptr)
        {
            _M_root = node;
        }

        return *this;
    }

    ImGuiAdditionalWindowList::~ImGuiAdditionalWindowList()
    {
        while (_M_root)
        {
            destroy(_M_root);
        }
    }

    static Window* _M_current_window = nullptr;

    Window::Window(Engine::Window* window, ImGuiContext* ctx) : _M_context(ctx), _M_window(window)
    {}

    void Window::free_resources()
    {
        while (!_M_textures.empty())
        {
            release_texture(*_M_textures.begin());
        }
    }

    ImGuiContext* Window::context() const
    {
        return _M_context;
    }

    ImDrawData* Window::draw_data()
    {
        return _M_draw_data.draw_data();
    }

    Window& Window::new_frame()
    {
        ImGui::SetCurrentContext(_M_context);
        _M_window->interface()->new_imgui_frame();

        RHI* rhi = engine_instance->rhi();
        rhi->imgui_new_frame(_M_context);
        ImGui::NewFrame();

        _M_current_window = this;
        return *this;
    }

    Window& Window::end_frame()
    {
        ImGui::SetCurrentContext(_M_context);
        window_list.render(_M_window->render_viewport());
        ImGui::Render();
        _M_current_window = nullptr;
        return *this;
    }

    Window& Window::prepare_render()
    {
        ImGui::SetCurrentContext(_M_context);
        _M_draw_data.copy(ImGui::GetDrawData());
        return *this;
    }

    Window& Window::render()
    {
        RHI* rhi = engine_instance->rhi();
        rhi->imgui_render(_M_context, draw_data());
        return *this;
    }

    ImGuiTexture* Window::create_texture()
    {
        ImGuiTexture* texture = new ImGuiTexture();
        _M_textures.insert(texture);
        return texture;
    }

    Window& Window::release_texture(ImGuiTexture* texture)
    {
        if (_M_textures.contains(texture))
        {
            _M_textures.erase(texture);
            delete texture;
        }

        return *this;
    }

    Window* Window::current()
    {
        return _M_current_window;
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
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered())
            {
                ImGui::CloseCurrentPopup();
                status = false;
            }

            ImGui::EndPopup();
        }

        return status;
    }
}// namespace Engine::ImGuiRenderer
