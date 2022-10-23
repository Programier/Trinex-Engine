#pragma once
#include <Core/engine.hpp>
#include <ImGui/imgui.h>
#include <vector>
#include <list>
#include <Window/window.hpp>
#include <panel.hpp>

namespace Engine
{
    class Editor
    {
        ImGuiIO* io = nullptr;
        ImGuiStyle* style = nullptr;
        bool _M_resized = true;
        Panel* _M_toolbar = nullptr;
        std::vector<Panel*> _M_panels;
        std::vector<Panel*> _M_additional_panels;


    private:
        void render_frame();
        void render_gui();
        static void on_resize();

    public:
        Editor();
        void push_panel(Panel*);
        Editor& loop();
        ~Editor();
    };

    extern Editor* editor;
    extern Window window;
    extern const Size2D& window_size;
}// namespace Engine
