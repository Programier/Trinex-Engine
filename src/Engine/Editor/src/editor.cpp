#include <Core/logger.hpp>
#include <ImGui/imgui_init.hpp>
#include <Window/window.hpp>
#include <editor.hpp>
#include <toolbar.hpp>


namespace Engine
{
    Editor* editor = nullptr;
    Window window;
    const Size2D& window_size = window.size();


    class LeftPanel : public Panel
    {
    public:
        LeftPanel()
        {
            name = "LeftPanel";
        }

        void render() override
        {
            ImGui::BeginChild(name.c_str(), {0, 0}, true);


            ImGui::EndChild();
        }

        ~LeftPanel()
        {}
    };


    class CentralPanel : public Panel
    {
    public:
        CentralPanel()
        {
            name = "CentralPanel";
        }

        void render() override
        {
            ImGui::BeginChild(name.c_str(), {0, 0}, true);


            ImGui::EndChild();
        }

        ~CentralPanel()
        {}
    };


    class RightPanel : public Panel
    {
    public:
        RightPanel()
        {
            name = "RightPanel";
        }

        void render() override
        {
            ImGui::BeginChild(name.c_str(), {0, 0}, true);
            ImGui::EndChild();
        }

        ~RightPanel()
        {}
    };


    void Editor::on_resize()
    {
        editor->_M_resized = true;
    }

    Editor::Editor()
    {
        editor = this;
        init();
        window.init({1280, 720}, "Engine Editor", Engine::WindowAttrib::WIN_RESIZABLE);


        // Init GUI
        const char* glsl_ver = "#version 300 es";
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        // Setup Platform/Renderer backends

        ImGuiInit::init(glsl_ver);

        io = &ImGui::GetIO();
        style = &ImGui::GetStyle();
        window.on_resize_callback(on_resize);

        _M_toolbar = new ToolBar;
        _M_panels = {new LeftPanel, new CentralPanel, new RightPanel};
        _M_additional_panels.reserve(50);
    }


    void Editor::render_frame()
    {}


    void Editor::render_gui()
    {
        static bool first_frame = true;
        ImGuiInit::new_frame();
        ImGui::NewFrame();

        ImGui::Begin("MainFrame", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
        if (first_frame)
        {
            ImGui::SetWindowPos({0, 0});
        }

        if (_M_resized)
        {
            ImGui::SetWindowSize({window.width(), window.height()});
        }

        _M_toolbar->render();

        ImGui::Columns(3);
        for (auto panel : _M_panels)
        {
            panel->render();
            ImGui::NextColumn();
        }


        for (auto panel : _M_additional_panels)
        {
            panel->render();
        }

        ImGui::End();
        ImGui::EndFrame();
        ImGui::Render();
        ImGuiInit::render();

        first_frame = false;
        _M_resized = false;
    }

    Editor& Editor::loop()
    {
        while (window.is_open())
        {
            window.bind().clear_buffer();
            render_frame();
            render_gui();
            window.swap_buffers();
            window.event.poll_events();
        }
        return *this;
    }

    void Editor::push_panel(Panel* panel)
    {
        _M_additional_panels.push_back(panel);
    }

    Editor::~Editor()
    {
        ImGuiInit::terminate_imgui();
        ImGui::DestroyContext();
        editor = nullptr;
        delete _M_toolbar;
        for (auto panel : _M_panels) delete panel;
    }
}// namespace Engine
