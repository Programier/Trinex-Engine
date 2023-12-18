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

    Window::Window(WindowInterface* interface, ImGuiContext* ctx) : _M_context(ctx), _M_interface(interface)
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
        _M_interface->new_imgui_frame();

        RHI* rhi = engine_instance->rhi();
        rhi->imgui_new_frame(_M_context);
        ImGui::NewFrame();

        return *this;
    }

    Window& Window::end_frame()
    {
        ImGui::SetCurrentContext(_M_context);
        ImGui::Render();
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

    bool InputText(const char* label, String& buffer, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        InputTextCallback data;
        data.callback = callback;
        data.userdata = user_data;
        data.str      = &buffer;

        flags |= ImGuiInputTextFlags_CallbackResize;
        return ImGui::InputText(label, buffer.data(), buffer.size() + 1, flags, input_text_callback, &data);
    }
}// namespace Engine::ImGuiRenderer
