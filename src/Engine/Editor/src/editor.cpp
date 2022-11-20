#include <Core/init.hpp>
#include <Graphics/enable_param.hpp>
#include <ImGui/imgui.h>
#include <ImGui/imgui_init.hpp>
#include <editor.hpp>
#include <left_panel.hpp>
#include <menu_bar.hpp>
#include <right_panel.hpp>
#include <viewport.hpp>


namespace Editor
{
    Application::Application()
    {
        Engine::init(Engine::EngineAPI::OpenGL);
        _M_window.init({1280, 720}, "Editor", Engine::WindowAttrib::WIN_RESIZABLE);

        // Init GUI
        const char* glsl_ver = "#version 300 es";
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        Engine::ImGuiInit::init(glsl_ver);

        _M_panels = {new MenuBar, new LeftPanel, new ViewPort, new RightPanel};
        Engine::enable(Engine::EnableCap::DepthTest)(Engine::EnableCap::Blend);
        Engine::blend_func(Engine::BlendFunc::SrcAlpha, Engine::BlendFunc::OneMinusSrcAlpha);
    }

    Application& Application::loop()
    {
        while (_M_window.is_open())
        {
            _M_window.clear_buffer();

            Engine::ImGuiInit::new_frame();
            ImGui::NewFrame();

            ImGui::Begin("MainFrame", nullptr,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                 ImGuiWindowFlags_NoSavedSettings);

            ImGui::SetWindowPos({0, 0});
            ImGui::SetWindowSize({_M_window.width(), _M_window.height()});
            _M_panels[0]->render();

            ImGui::Columns(3);
            static int is_first_frame = 2;
            if (is_first_frame)
            {
                ImGui::SetColumnWidth(0, _M_window.width() / 5.f);
                ImGui::SetColumnWidth(1, (_M_window.width() / 5.f) * 3.f);
                ImGui::SetColumnWidth(2, _M_window.width() / 5.f);
                is_first_frame--;
            }


            for (auto it = ++_M_panels.begin(); it != _M_panels.end(); ++it)
            {
                (*it)->render();
                ImGui::NextColumn();
            }

            ImGui::End();
            ImGui::EndFrame();
            ImGui::Render();
            Engine::ImGuiInit::render();

            _M_window.swap_buffers().event.poll_events();
        }

        return *this;
    }

    Application::~Application()
    {
        for (auto panel : _M_panels) delete panel;
    }
}// namespace Editor
