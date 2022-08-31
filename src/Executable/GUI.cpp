#include <Application.hpp>
#include <BasicFunctional/engine_types.hpp>
#include <GUI.hpp>
#include <ImGui/ImGuiFileBrowser.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_init.hpp>
#include <Init/init.hpp>
#include <Window/monitor.hpp>
#include <application_debug.hpp>
#include <glm/ext.hpp>
#include <iostream>


using namespace Engine;

// Prototypes
static void render_menu_bar();
static void render_left_panel();
static void render_right_panel();
static void render_viewport();

static Engine::Application* app = nullptr;
ImGuiIO* io = nullptr;


static struct Panel {
    ImVec2 size = {0., 0.};
    ImVec2 position;
    bool is_open = true;
    std::string name;
    void (*render)() = nullptr;

    Panel(const std::string& name = "") : name(name)
    {}
} LeftPanel("Left Panel"), RightPanel("Right Panel"), ViewPort("ViewPort");

struct {
    bool fullscren_mode = false;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    std::size_t frame = 0;
    std::size_t active_panels = 3;
    Panel* panels[3] = {&LeftPanel, &RightPanel, &ViewPort};
    Size2D window_size;
    bool vsync = true;
    float widget_size = 1.5f;

    const float dpi_mult_value = 0.013025f;
    imgui_addons::ImGuiFileBrowser file_dialog;
    bool open_file = false;
} GUI_data;


void GUI::init(Engine::Application* _app)
{
    app = _app;
#ifdef __ANDROID__
    const char* glsl_ver = "#version 300 es";
#else
    const char* glsl_ver = "#version 330";
#endif
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends

    ImGuiInit::init(glsl_ver);

    io = &ImGui::GetIO();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //Font.font = imgui_load_font();

    LeftPanel.size = {app->window_size.x / 5, app->window_size.y};
    RightPanel.size = LeftPanel.size;

    LeftPanel.render = render_left_panel;
    RightPanel.render = render_right_panel;
    ViewPort.render = render_viewport;

    GUI_data.window_size = app->window.size();

    std::clog << Monitor::dpi().ddpi << std::endl;
    GUI_data.widget_size = Monitor::dpi().ddpi * GUI_data.dpi_mult_value;
}


void GUI::terminate()
{
    ImGuiInit::terminate();
    ImGui::DestroyContext();
}


void change_size(Panel& panel)
{
    if (!panel.is_open)
    {
        panel.size = {0, 0};
        return;
    }

    if (GUI_data.frame)
        panel.size.x = std::min(ImGui::GetWindowSize().x, app->window_size.x / static_cast<float>(GUI_data.active_panels));
    panel.size.y = app->window_size.y;
}

static void render_menu_bar()
{
    /////////////////////////   Menu   /////////////////////////
    static bool render_about = false;
    ImGui::BeginMainMenuBar();
    ImGui::SetWindowFontScale(GUI_data.widget_size);
    ImGui::SetNextWindowSize({350, 0});
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Open", "Open model"))
        {
            GUI_data.open_file = true;
        }

        if (ImGui::MenuItem("Close", "Close the Engine"))
            app->window.close();

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        for (auto panel : GUI_data.panels)
        {
            if (ImGui::Checkbox(panel->name.c_str(), &panel->is_open))
            {
                GUI_data.active_panels += (-1 + (cast(int, panel->is_open) << 1));
            }
        }

        ImGui::EndMenu();
    }


    ImGui::SetNextWindowSize({350, 0});
    if (ImGui::BeginMenu("Edit"))
    {
        ImGui::Button("Hello");
        ImGui::EndMenu();
    }
    ImGui::SetNextWindowSize({350, 0});
    if (ImGui::BeginMenu("Help"))
    {
        if (ImGui::MenuItem("Author", "Basic info about author"))
            render_about = true;

        ImGui::EndMenu();
    }

    if (render_about)
    {
        ImGui::Begin("About", &render_about,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowFocus();
        ImGui::SetWindowSize({280, 78});
        auto tmp = (app->window_size / 2.f) - Size2D({140.f, 39.f});
        ImGui::SetWindowPos({tmp.x, tmp.y});
        ImGui::Text("Vladyslav Reminskyi - Programmer\n@Programier - Telegram");

        ImGui::End();
    }


    LeftPanel.position.y = RightPanel.position.y = ViewPort.position.y = ImGui::GetWindowHeight();
    ImGui::EndMainMenuBar();
}


void render_left_panel()
{
    /////////////////////////   Left PANEL  /////////////////////////

    ImGui::Begin("Left Panel", 0, GUI_data.flags);
    ImGui::SetWindowFontScale(GUI_data.widget_size);
    change_size(LeftPanel);
    ImGui::SetWindowSize(LeftPanel.size);
    ImGui::SetWindowPos(LeftPanel.position);

    if (ImGui::CollapsingHeader("Window"))
    {
        if (ImGui::ColorEdit4("Color", glm::value_ptr(app->background_color)))
        {
            std::clog << app->background_color << std::endl;
            app->window.background_color(app->background_color);
        }

        if (ImGui::InputText("Title", app->window_title, 100))
            app->window.title(app->window_title);

#ifndef __ANDROID__

        if (ImGui::Checkbox("Fullscren", &GUI_data.fullscren_mode))
        {
            app->window.attribute(WIN_FULLSCREEN_DESKTOP, GUI_data.fullscren_mode);
            GUI_data.window_size = app->window.event.poll_events().size();
        }

        if (!GUI_data.fullscren_mode)
        {
            bool changed = ImGui::SliderFloat("Width", &GUI_data.window_size.x, 100, Engine::Monitor::width());
            changed = changed || ImGui::SliderFloat("Height", &GUI_data.window_size.y, 100, Engine::Monitor::height() - 50);
            if (changed)
            {
                app->window.size(GUI_data.window_size).event.poll_events();
            }
        }
#endif

        ImGui::SliderFloat("Widget size", &GUI_data.widget_size, 0.1f, 4.f);

        if (ImGui::Checkbox("Vsync", &GUI_data.vsync))
            app->window.vsync(GUI_data.vsync);
    }

    ImGui::End();
}

void render_right_panel()
{
    /////////////////////////   RIGHT PANEL  /////////////////////////

    ImGui::Begin("Right Panel", 0, GUI_data.flags);
    ImGui::SetWindowFontScale(GUI_data.widget_size);
    change_size(RightPanel);
    RightPanel.position.x = app->window_size.x - RightPanel.size.x;
    ImGui::SetWindowPos(RightPanel.position);
    ImGui::SetWindowSize(RightPanel.size);
    ImGui::End();
}

void render_viewport()
{
    /////////////////////////   VIEW PORT /////////////////////////

    ImGui::Begin("ViewPort", 0, GUI_data.flags);

    ImGui::SetWindowFontScale(GUI_data.widget_size);
    ViewPort.size.x = app->window_size.x - LeftPanel.size.x - RightPanel.size.x;
    ViewPort.size.y = app->window_size.y;
    ImGui::SetWindowSize(ViewPort.size);
    ViewPort.position.x = LeftPanel.size.x;
    ImGui::SetWindowPos(ViewPort.position);
    ImGui::End();
}


void GUI::render()
{
    ImGuiInit::new_frame();
    ImGui::NewFrame();


    render_menu_bar();

    for (auto panel : GUI_data.panels)
        if (panel->is_open)
            panel->render();
        else if (panel != &ViewPort)
            change_size(*panel);

    if (GUI_data.open_file)
    {
        ImGui::OpenPopup("Open File");
        GUI_data.open_file = false;
    }

    if (GUI_data.file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310)))
    {
        std::cout << GUI_data.file_dialog.selected_fn << std::endl;
        std::cout << GUI_data.file_dialog.selected_path << std::endl;
    }

    ImGui::Render();
    ImGuiInit::render();
    GUI_data.frame++;
}
