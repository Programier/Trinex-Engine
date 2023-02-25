#pragma once
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>
#include <Window/window.hpp>

namespace Engine
{
    ENGINE_EXPORT class Application
    {
    private:
    protected:
        struct {
            String window_name;
            Size2D window_size;
            uint16_t window_attribs = WindowAttrib::WinShown;
            EngineAPI api = EngineAPI::OpenGL;
        } init_info;

        CallBack<const Event&> _M_update_event_callback;

    public:
        Window window;


        delete_copy_constructors(Application);
        Application();
        Application& init();

        virtual Application& on_init();
        virtual Application& update_logic();
        virtual Application& on_render_frame();
        Application& start();
        virtual ~Application();
    };
}// namespace Engine
