#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Graphics/render_viewport.hpp>
#include <imgui.h>


namespace ImGui
{
    class FileBrowser;
}

namespace Engine
{
    class Window;
    struct WindowInterface;
    struct RHI_ImGuiTexture;
    class Texture;
    class Sampler;
    class RenderViewport;
    class Pipeline;
}// namespace Engine


namespace Engine::ImGuiHelpers
{
    template<typename OutType, typename InType>
    OutType construct_vec2(const InType& in)
    {
        OutType out;
        out.x = in.x;
        out.y = in.y;
        return out;
    }

    template<typename OutType, typename InType>
    OutType construct_vec4(const InType& in)
    {
        OutType out;
        out.x = in.x;
        out.y = in.y;
        out.z = in.z;
        out.w = in.w;
        return out;
    }
}// namespace Engine::ImGuiHelpers

namespace Engine::ImGuiRenderer
{
    ENGINE_EXPORT ImTextureID create_fonts_texture(const byte* fonts_texture_data, float fonts_texture_x, float fonts_texture_y);

    ENGINE_EXPORT extern float rhi_rendering_scale_factor;

    class ENGINE_EXPORT DrawData final
    {
        ImDrawData m_draw_data[2];
        byte m_logic_index  = 0;
        byte m_render_index = 0;

    public:
        ImDrawData* draw_data();
        DrawData& release(bool full = false);
        DrawData& copy(ImDrawData* draw_data);

        DrawData& swap_render_index();
        DrawData& swap_logic_index();

        ~DrawData();
    };


    class ENGINE_EXPORT ImGuiAdditionalWindowList;

    class ENGINE_EXPORT ImGuiAdditionalWindow
    {
    public:
        size_t frame_number = 0;
        bool closable       = true;
        CallBacks<void()> on_close;

        ImGuiAdditionalWindow();
        delete_copy_constructors(ImGuiAdditionalWindow);

        virtual void init(RenderViewport* viewport);
        virtual bool render(RenderViewport* viewport) = 0;
        FORCE_INLINE virtual ~ImGuiAdditionalWindow(){};
    };


    class ENGINE_EXPORT ImGuiAdditionalWindowList final
    {
        struct Node {
            ImGuiAdditionalWindow* window = nullptr;
            Node* next                    = nullptr;
            Node* parent                  = nullptr;
            const void* id                = nullptr;
        };

        Node* m_root = nullptr;

        ImGuiAdditionalWindowList& push(ImGuiAdditionalWindow* window, const void* id = nullptr);
        Node* destroy(Node* node);

        Node* close_window_internal(Node* node);

    public:
        ImGuiAdditionalWindowList() = default;
        delete_copy_constructors(ImGuiAdditionalWindowList);

        template<typename Type, typename... Args>
        Type* create(Args&&... args)
        {
            Type* instance = new Type(std::forward<Args>(args)...);
            push(instance, nullptr);
            return instance;
        }

        template<typename Type, typename... Args>
        Type* create_identified(const void* id, Args&&... args)
        {
            if (id == nullptr)
                return create<Type>(std::forward<Args>(args)...);

            for (Node* node = m_root; node; node = node->next)
            {
                if (node->id == id)
                {
                    return reinterpret_cast<Type*>(node->window);
                }
            }

            Type* instance = new Type(std::forward<Args>(args)...);
            push(instance, id);
            return instance;
        }

        ImGuiAdditionalWindowList& close_window(class ImGuiAdditionalWindow* window);
        ImGuiAdditionalWindowList& close_all_windows();
        ImGuiAdditionalWindowList& render(class RenderViewport* viewport);
        ~ImGuiAdditionalWindowList();
    };


    class ENGINE_EXPORT Window final : private Engine::Object
    {
        declare_class(Window, Object);

    private:
        DrawData m_draw_data;
        size_t m_frame;

        ImGuiContext* m_context;
        Engine::Window* m_window;

        Window(Engine::Window* window, ImGuiContext* context);
        Window& free_resources();
        ~Window();

    public:
        ImGuiAdditionalWindowList window_list;
        CallBacks<void()> on_destroy;

        Window(const Window& window)     = delete;
        Window& operator=(const Window&) = delete;

        ImGuiContext* context() const;
        ImDrawData* draw_data();
        Window& new_frame();
        Window& end_frame();
        Window& rhi_render();
        Engine::Window* window() const;
        size_t frame_index() const;
        Window& reset_frame_index();
        static Window* current();
        static void make_current(Window*);

        friend class Engine::Window;
        friend class Engine::Object;
    };


    bool ENGINE_EXPORT IsMouseDownNow(ImGuiMouseButton button);

    bool ENGINE_EXPORT InputText(const char* label, String& buffer, ImGuiInputTextFlags flags = 0,
                                 ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

    bool ENGINE_EXPORT InputTextMultiline(const char* label, String& buffer, const ImVec2& size = ImVec2(0.0f, 0.0f),
                                          ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
                                          void* user_data = nullptr);

    bool ENGINE_EXPORT InputTextWithHint(const char* label, const char* hint, String& buffer, ImGuiInputTextFlags flags = 0,
                                         ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

    bool ENGINE_EXPORT BeginPopup(const char* name, ImGuiWindowFlags flags = 0, bool (*callback)(void*) = nullptr,
                                  void* userdata = nullptr);

    bool ENGINE_EXPORT IsWindowRectHovered();


    template<typename Instance>
    FORCE_INLINE bool BeginPopup(const char* name, ImGuiWindowFlags flags = 0,
                                 bool (Instance::*callback)(void* userdata) = nullptr, Instance* instance = nullptr,
                                 void* userdata = nullptr)
    {
        struct InternalData {
            bool (Instance::*callback)(void*) = nullptr;
            Instance* instance                = nullptr;
            void* userdata                    = nullptr;
        } data;

        data.callback = callback;
        data.instance = instance;
        data.userdata = userdata;

        struct InternalFunction {
            static bool execute(void* _userdata)
            {
                InternalData* data = reinterpret_cast<InternalData*>(_userdata);
                return (data->instance->*data->callback)(data->userdata);
            }
        };

        return BeginPopup(name, flags, InternalFunction::execute, &data);
    }
}// namespace Engine::ImGuiRenderer
