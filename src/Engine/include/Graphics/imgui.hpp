#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <imgui.h>

namespace Engine
{
    class Window;
    struct WindowInterface;
    struct RHI_ImGuiTexture;
    class Texture;
    class Sampler;
}// namespace Engine

namespace Engine::ImGuiRenderer
{
    class ENGINE_EXPORT DrawData final
    {
        ImDrawData _M_draw_data;

    public:
        ImDrawData* draw_data();
        DrawData& release();
        DrawData& copy(ImDrawData* draw_data);

        ~DrawData();
    };

    class ImGuiTexture final
    {
    private:
        RHI_ImGuiTexture* _M_handle = nullptr;

        ImGuiTexture();
        ~ImGuiTexture();
        ImGuiTexture& release();

    public:
        delete_copy_constructors(ImGuiTexture);
        void* handle() const;
        ImGuiTexture& init(ImGuiContext* ctx, Texture* texture, Sampler* sampler);

        friend class Window;
    };

    class ENGINE_EXPORT Window final
    {
    private:
        DrawData _M_draw_data;
        Set<ImGuiTexture*> _M_textures;

        ImGuiContext* _M_context;
        WindowInterface* _M_interface;


        Window(WindowInterface* interface, ImGuiContext* context);
        void free_resources();
        ~Window();

    public:
        Window(const Window& window)     = delete;
        Window& operator=(const Window&) = delete;

        ImGuiContext* context() const;
        ImDrawData* draw_data();
        Window& new_frame();
        Window& end_frame();
        Window& prepare_render();
        Window& render();

        ImGuiTexture* create_texture();
        Window& release_texture(ImGuiTexture*);

        friend class Engine::Window;
    };

    bool InputText(const char* label, String& buffer, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
                   void* user_data = nullptr);
}// namespace Engine::ImGuiRenderer
