#include <Application.hpp>
#include <Core/engine_types.hpp>
#include <Core/init.hpp>
#include <GUI.hpp>
#include <ImGui/ImGuiFileDialog.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_init.hpp>
#include <LibLoader/lib_loader.hpp>
#include <Sensors/sensor.hpp>
#include <Window/monitor.hpp>
#include <application_debug.hpp>
#include <glm/ext.hpp>
#include <iostream>

#include <Core/logger.hpp>

static std::list<std::string> _M_logs;

class GUI_Logger : public Engine::Logger
{
public:
    GUI_Logger& log(const char* format, ...) override
    {
        va_list args;
        va_start(args, format);
        char buffer[1024];

        vsprintf(buffer, format, args);
        Engine::standart_logger().log("%s", buffer);
        va_end(args);
        _M_logs.push_back(buffer);
        return *this;
    }
} gui_logger;


struct Panel {
    void (*render_func)() = nullptr;
    bool render = true;
    const char* name;
};


using namespace Engine;

static Engine::Application* app = nullptr;
ImGuiIO* io = nullptr;

struct {
    bool fullscren_mode = false;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar;
    std::size_t frame = 0;
    std::size_t active_panels = 3;
    const Engine::Size2D* _M_engine_win_size = nullptr;
    bool vsync = true;
    float font_scale = 1.5f;
    const float dpi_mult_value = 0.013025f;
    Engine::Sensor sensor;
    ImVec2 _M_window_size;

    Panel panels[3];
    char window_title[100];
    bool buttons = false;
    float button_size_k = 1.f;
    bool render_depth = false;
    Size2D view_offset;
} GUI_data;


void left_pane();
void right_pane();
void viewport_pane();

void GUI::init_logger()
{
    Engine::logger = &gui_logger;
}

void GUI::init(Engine::Application* _app)
{
    app = _app;
#ifdef __ANDROID__
    const char* glsl_ver = "#version 300 es";
#else
    const char* glsl_ver = "#version 300 es";
#endif
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends

    ImGuiInit::init(glsl_ver);

    io = &ImGui::GetIO();
    ImGuiIO& io = ImGui::GetIO();
    io.WantCaptureKeyboard = true;
    io.ConfigFlags = ImGuiConfigFlags_IsTouchScreen | ImGuiConfigFlags_NoMouseCursorChange;

    //Font.font = imgui_load_font();


    GUI_data._M_engine_win_size = &app->window.size();
    GUI_data.font_scale = Monitor::dpi().ddpi * GUI_data.dpi_mult_value;
    GUI_data.sensor.open(1);


    // Init panels structure
    GUI_data.panels[0].render_func = left_pane;
    GUI_data.panels[1].render_func = viewport_pane;
    GUI_data.panels[2].render_func = right_pane;

    GUI_data.panels[0].name = "Left panel";
    GUI_data.panels[1].name = "ViewPort";
    GUI_data.panels[2].name = "Right pane";

    strcpy(GUI_data.window_title, app->window.title().c_str());
    init_logger();
}

void GUI::terminate()
{
    ImGuiInit::terminate_imgui();
    ImGui::DestroyContext();
    Engine::logger = &Engine::standart_logger();
}


static void scroll_event(float value = 5.f)
{
    auto fingers = app->window.event.touchscreen.fingers_count();
    if (fingers < 2)
        return;

    float scroll_y = 0;
    for (unsigned int i = 0; i < fingers; i++) scroll_y += app->window.event.touchscreen.get_finger(i).offset.y;

    scroll_y /= static_cast<float>(fingers);
    //scroll_y = scroll_y < 0 ? -value : value;
    io->AddMouseWheelEvent(0.f, scroll_y * value);
}


void menu_bar()
{
    /////////////////////////   Menu   /////////////////////////
    static bool render_about = false;
    ImGui::BeginMainMenuBar();

    ImGui::SetNextWindowSize({350, 0});
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Open", "Open model"))
        {
#ifdef __ANDROID__
            static const char* path = "/sdcard/";
#else
            static const char* path = ".";
#endif
            ImGuiFileDialog::Instance()->OpenDialog("OpenFile", "Open File", "*.*", path);
        }

        if (ImGui::MenuItem("Open skybox", "skybox"))
        {
#ifdef __ANDROID__
            static const char* path = "/sdcard/";
#else
            static const char* path = ".";
#endif
            ImGuiFileDialog::Instance()->OpenDialog("OpenSkybox", "Open Skybox", "*.*", path);
        }


        if (ImGui::MenuItem("Close", "Close the Engine"))
            app->window.close();

        ImGui::EndMenu();
    }


    if (ImGuiFileDialog::Instance()->IsOpened("OpenFile"))
    {
        ImVec2 _M_size(app->window.width(), app->window.height());
        ImGui::SetNextWindowPos({0, 0});
        scroll_event();
        if (ImGuiFileDialog::Instance()->Display(
                    "OpenFile", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse, _M_size,
                    _M_size))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                app->load_scene(ImGuiFileDialog::Instance()->GetFilePathName());
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    else if (ImGuiFileDialog::Instance()->IsOpened("OpenSkybox"))
    {
        ImVec2 _M_size(app->window.width(), app->window.height());
        ImGui::SetNextWindowPos({0, 0});
        scroll_event();
        if (ImGuiFileDialog::Instance()->Display(
                    "OpenSkybox", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse, _M_size,
                    _M_size))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                app->load_skybox(ImGuiFileDialog::Instance()->GetFilePathName());
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }

    if (ImGui::BeginMenu("View"))
    {
        for (auto& panel : GUI_data.panels)
        {
            ImGui::Checkbox(panel.name, &panel.render);
        }

        ImGui::Checkbox("Buttons", &GUI_data.buttons);
        ImGui::Checkbox("Depth buffer", &GUI_data.render_depth);
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
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFocus();
        auto tmp = (app->window_size / 2.f) - Size2D({ImGui::GetWindowSize().x / 2.f, ImGui::GetWindowSize().y / 2.f});
        ImGui::SetWindowPos({tmp.x, tmp.y});
        ImGui::Text("Vladyslav Reminskyi - Programmer\n@Programier - Telegram");

        ImGui::End();
    }

    ImGui::EndMainMenuBar();
}

void left_pane()
{
    float pos = ImGui::GetCursorPosY();
    ImGui::BeginChild("##left_pane", {ImGui::GetColumnWidth(0), GUI_data._M_window_size.y - pos}, false,
                      ImGuiWindowFlags_NoSavedSettings);


    /////////////////////////   Left PANEL  /////////////////////////

    ImGui::Text("FPS: %.2f\n", io->Framerate);
    ImGui::NewLine();

    if (ImGui::CollapsingHeader("Window"))
    {
        if (ImGui::InputText("Title", GUI_data.window_title, 100))
            app->window.title(GUI_data.window_title);

#ifndef __ANDROID__

        if (ImGui::Checkbox("Fullscreen", &GUI_data.fullscren_mode))
        {
            app->window.attribute(WIN_FULLSCREEN_DESKTOP, GUI_data.fullscren_mode);
            app->window.event.poll_events();
        }
#endif

        ImGui::DragScalarN("Size", ImGuiDataType_Float, (void*) glm::value_ptr(app->window.size()), 2, 0);
        ImGui::SliderFloat("Font size", &GUI_data.font_scale, 0.1f, 4.f);

        ImGui::SliderFloat("Virtual button size", &GUI_data.button_size_k, 0.1f, 4.f);

        if (ImGui::Checkbox("Vsync", &GUI_data.vsync))
            app->window.vsync(GUI_data.vsync);
    }

    ImGui::NewLine();

    if (ImGui::CollapsingHeader("Scene"))
    {
        if (ImGui::InputFloat3("Scene scale", glm::value_ptr(app->model_scale)))
        {
            app->update_model_matrix();
        }

        ImGui::SliderFloat("Speed", &app->speed, 0.01f, 50.f);
        ImGui::SliderFloat("Min Render", &app->camera->min_render_distance(), 0.00001f, 1.f);
        ImGui::SliderFloat("Max Render", &app->camera->max_render_distance(), 10, 1000.f);
        ImGui::DragFloat3("POS", (float*) glm::value_ptr(app->camera->position()), 0.f);
        ImGui::DragFloat3("Angles", (float*) glm::value_ptr(glm::degrees(app->camera->euler_angles())), 0.f);
        static const float pi = 2 * glm::pi<float>();
        ImGui::SliderFloat("View angle", &app->camera->viewing_angle(), 0, pi);

        static int value = (int) app->skybox.min_filter();
        static int value2 = (int) app->skybox.mag_filter();
        if (ImGui::SliderInt("MIN", &value, 0, 5))
        {
            app->skybox.min_filter((TextureFilter) (unsigned int) value);
        }

        if (ImGui::SliderInt("MAG", &value2, 0, 5))
        {
            auto f = (TextureFilter) (unsigned int) value2;
            app->skybox.mag_filter(f);
        }

        //        static Size2D texture_size = app->texture_2D.size();
        //        if (ImGui::SliderFloat2("Texture size", glm::value_ptr(texture_size), 1, 8000))
        //        {
        //            app->generate_texture(texture_size);
        //            value = (int) app->texture_2D.min_filter();
        //            value2 = (int) app->texture_2D.mag_filter();
        //        }

        ImGui::Text("Offset: {%.3f : %.3f}", MouseEvent::offset().x, MouseEvent::offset().y);
    }


    ImGui::EndChild();
}

void viewport_pane()
{
    float width = ImGui::GetColumnWidth(1);
    ImGui::BeginChild("viewport_pane", {width, GUI_data._M_window_size.y - 50}, false, ImGuiWindowFlags_NoSavedSettings);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    app->viewport_size = {width, GUI_data._M_engine_win_size->y};
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    auto tmp = ImVec2(pos.x + width, GUI_data._M_window_size.y);

    GUI_data.view_offset = app->viewport_size / app->monitor_size;

    drawList->AddImage((void*) app->view_image.internal_id(), pos, tmp, ImVec2(0, GUI_data.view_offset.y),
                       ImVec2(GUI_data.view_offset.x, 0));

    ImGui::EndChild();
}

void right_pane()
{
    float pos = ImGui::GetCursorPosY();
    ImGui::BeginChild("##right_pane", {ImGui::GetColumnWidth(2), GUI_data._M_window_size.y - pos}, false,
                      ImGuiWindowFlags_NoSavedSettings);
    for (auto& text : _M_logs) ImGui::Text("%s", text.c_str());

    ImGui::EndChild();
}

void render_buttons()
{
    ImGui::Begin("Buttons", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    float win_size = 250.f * GUI_data.button_size_k;
    float button_size = win_size / 3.f;
    auto b_size = ImVec2(button_size, button_size);
    ImGui::SetWindowSize({win_size + 20, button_size * 5});
    ImGui::Columns(3, nullptr, false);
    ImGui::NextColumn();

    for (int i = 0; i < 2; i++) ImGui::SetColumnWidth(i, win_size / 3);

    struct VirtualKey {
        const char* name;
        Key key;
        unsigned int prev_status = 0;
    };

    static VirtualKey virtual_keys[4] = {{"W", KEY_W, 0}, {"A", KEY_A, 0}, {"D", KEY_D, 0}, {"S", KEY_S, 0}};

    ImGui::PushButtonRepeat(true);

    for (auto& vkey : virtual_keys)
    {
        if (ImGui::Button(vkey.name, b_size))
        {
            if (!Engine::KeyboardEvent::pressed(vkey.key) && vkey.prev_status == 0)
                Engine::KeyboardEvent::push_event(vkey.key, Engine::KeyStatus::PRESSED);
            vkey.prev_status = 2;
        }
        else if (Engine::KeyboardEvent::pressed(vkey.key))
        {
            if (vkey.prev_status == 0)
                Engine::KeyboardEvent::push_event(vkey.key, Engine::KeyStatus::RELEASED);
            vkey.prev_status >>= 1;
        }


        ImGui::NextColumn();
        ImGui::NextColumn();
    }


    ImGui::Columns(2);

    static VirtualKey virtual_keys2[2] = {{"UP", KEY_UP, 0}, {"DOWN", KEY_DOWN, 0}};

    for (auto& vkey : virtual_keys2)
    {
        ImGui::NewLine();

        if (ImGui::Button(vkey.name, b_size))
        {
            if (!Engine::KeyboardEvent::pressed(vkey.key) && vkey.prev_status == 0)
                Engine::KeyboardEvent::push_event(vkey.key, Engine::KeyStatus::PRESSED);
            vkey.prev_status = 2;
        }
        else if (Engine::KeyboardEvent::pressed(vkey.key))
        {
            if (vkey.prev_status == 0)
                Engine::KeyboardEvent::push_event(vkey.key, Engine::KeyStatus::RELEASED);
            vkey.prev_status >>= 1;
        }


        ImGui::NextColumn();
    }

    ImGui::PopButtonRepeat();
    ImGui::End();
}


void render_depth()
{
    ImGui::Begin("Depth buffer", nullptr, ImGuiWindowFlags_NoSavedSettings);

    if (GUI_data.frame == 0)
        ImGui::SetWindowSize({100, 100});

    auto size = ImGui::GetWindowSize();

    ImGui::Image(reinterpret_cast<void*>(app->depth_image.internal_id()), size, {0, GUI_data.view_offset.y},
                 {GUI_data.view_offset.x, 0});

    ImGui::End();
}


void tmp()
{
    ImGui::Begin("window", nullptr, GUI_data.flags);
    ImGui::SetWindowPos({0, 0});
    static auto pos = ImGui::GetCursorPosY();

    ImGui::GetFont()->Scale = GUI_data.font_scale;
    GUI_data._M_window_size.x = app->window.width();
    GUI_data._M_window_size.y = app->window.height();

    auto content_width = ImGui::GetWindowContentRegionWidth();
    ImGui::SetNextWindowContentSize({content_width, GUI_data._M_window_size.y - pos});


    ImGui::SetWindowSize(GUI_data._M_window_size);
    menu_bar();


    static const float mults[2] = {0.2, 0.6};
    ImGui::Columns(3, nullptr, true);

    if (GUI_data.frame == 1)
        for (int i = 0; i < 2; i++) ImGui::SetColumnWidth(i, content_width * mults[i]);


    for (auto panel : GUI_data.panels)
    {
        if (!panel.render)
        {
            ImGui::NextColumn();
            continue;
        }
        panel.render_func();
        ImGui::NextColumn();
    }

    ImGui::End();


    if (GUI_data.buttons)
        render_buttons();

    if (GUI_data.render_depth)
        render_depth();
}


void toolbar()
{
    ImGui::BeginMenuBar();


    static const char* names[] = {"File", "Edit", "Settings", "View", "About"};
    for (auto name : names)
    {
        if (ImGui::BeginMenu(name))
        {
            ImGui::EndMenu();
        }
    }

    ImGui::EndMenuBar();
}


void new_window()
{
    ImGui::Begin("Window", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetWindowPos({0, 0});
    ImGui::SetWindowSize({app->window_size.x, app->window_size.y});
    toolbar();

    ImGui::Columns(3, "Grid", true);

    for (int i = 0; i < 3; i++)
    {

        ImGui::BeginChild(std::to_string(i).c_str(), {0, 0}, true);

        for (int j = 0; j < 100; j++) ImGui::Button(std::string(std::to_string(i) + std::to_string(j)).c_str());
        ImGui::EndChild();
        ImGui::NextColumn();
    }

    ImGui::End();
}

void GUI::render()
{
    ImGuiInit::new_frame();
    ImGui::NewFrame();

    new_window();

    ImGui::EndFrame();
    ImGui::Render();
    ImGuiInit::render();
    GUI_data.frame++;
}
