#include <Core/engine_types.hpp>
#include <imgui.h>

namespace Engine
{
    class Window;
    struct WindowInterface;
}

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

    class ENGINE_EXPORT Window final
    {
    private:
        DrawData _M_draw_data;
        ImGuiContext* _M_context;
        WindowInterface* _M_interface;

        Window(WindowInterface* interface, ImGuiContext* context);
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

        friend class Engine::Window;
    };
}// namespace Engine::ImGuiRenderer
