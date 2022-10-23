#include <ImGui/imgui.h>
#include <editor.hpp>
#include <settings_window.hpp>
#include <toolbar.hpp>


#define proccessItem(item)                                                                                                  \
    if (ImGui::BeginMenu(##item))                                                                                           \
    {                                                                                                                       \
        _M_render_panel[PanelsType::item]->need_render = true;                                                              \
        ImGui::EndMenu();                                                                                                   \
    }

namespace Engine
{
    enum PanelsType : unsigned int
    {
        File,
        Edit,
        View,
        About,
        COUNT
    };


    class FilePanel : public Panel
    {
    public:
        void render() override
        {}
    };


    class AboutMessage : public Panel
    {
        std::string _M_text;
        std::string _M_title;
        bool _M_set_center = true;

    public:
        AboutMessage(const std::string& text, const std::string& title) : _M_text(text), _M_title(title)
        {
            need_render = false;
        }

        void render() override
        {
            if (!need_render)
                return;

            if (_M_set_center)
            {
                ImGui::SetNextWindowPos(ImVec2(window_size.x * 0.5f, window_size.y * 0.5f), ImGuiCond_Always,
                                        ImVec2(0.5f, 0.5f));
            }

            ImGui::Begin(_M_title.c_str(), &this->need_render,
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
            ImGui::Text("%s", _M_text.c_str());
            ImGui::End();

            _M_set_center = !need_render;
        }
    };


    class AboutWindow : public Panel
    {
        AboutMessage about_developper;
        AboutMessage about_engine;

    public:
        AboutWindow()
            : about_developper("Developer: Vladyslav Reminskyi\nTelegram: @Programier", "About Engine"),
              about_engine("Engine version: 1.0.0", "About Developer")
        {
            need_render = false;
            editor->push_panel(&about_developper);
            editor->push_panel(&about_engine);
        }

        void render() override
        {
            if (ImGui::MenuItem("About Engine"))
            {
                about_engine.need_render = true;
            }

            if (ImGui::MenuItem("About Developer"))
            {
                about_developper.need_render = true;
            }
        }
    };


    class EditPanel : public Panel
    {
        SettingsWindow settings;

    public:
        EditPanel()
        {
            editor->push_panel(&settings);
        }

        void render() override
        {
            if (ImGui::MenuItem("Settings", "Editor Settings"))
            {
                settings.need_render = true;
            }
        }
    };


    ToolBar::ToolBar()
    {
        name = "ToolBar";
        _M_render_panel.resize(PanelsType::COUNT);
        _M_render_panel[PanelsType::File] = new FilePanel();
        _M_render_panel[PanelsType::About] = new AboutWindow();
        _M_render_panel[PanelsType::Edit] = new EditPanel();
    }

    void ToolBar::render()
    {
        if (!need_render)
            return;

        ImGui::BeginMenuBar();
        if (ImGui::BeginMenu("File"))
        {
            _M_render_panel[PanelsType::File]->render();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            _M_render_panel[PanelsType::Edit]->render();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("About"))
        {
            _M_render_panel[PanelsType::About]->render();
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ToolBar::~ToolBar()
    {
        for (auto panel : _M_render_panel) delete panel;
    }
}// namespace Engine
