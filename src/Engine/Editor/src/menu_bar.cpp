#include <Core/logger.hpp>
#include <Graphics/textured_object.hpp>
#include <ImGui/ImGuiFileDialog.h>
#include <ImGui/imgui.h>
#include <Window/window.hpp>
#include <editor.hpp>
#include <menu_bar.hpp>
#include <resouces.hpp>
namespace Editor
{


#ifdef __ANDROID__
#define DEFAULT_PATH "/sdcard/"
#else
#define DEFAULT_PATH "./"
#endif

    void MenuBar::file_button()
    {
        auto file_choser_instance = ImGuiFileDialog::Instance();
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open file...", "Loading new model to scene"))
                file_choser_instance->OpenDialog("__open_model__", "Open file", "*.*", DEFAULT_PATH);
            ImGui::EndMenu();
        }

        if (file_choser_instance->IsOpened("__open_model__"))
        {
            ImGui::SetNextWindowPos({0, 0});
            ImGui::SetNextWindowSize({Engine::Window::width(), Engine::Window::height()});
            if (file_choser_instance->Display("__open_model__"))
            {
                if (file_choser_instance->IsOk())
                {
                    auto file = ImGuiFileDialog::Instance()->GetFilePathName();

                    try
                    {
                        Engine::StaticTexturedObject::load(file, &Resources::scene);
                    }
                    catch (const std::exception& e)
                    {
                        Engine::info_log("%s\n", e.what());
                    }
                }

                file_choser_instance->Close();
            }
        }
    }

    void MenuBar::view_button()
    {
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Show Depth Buffer"))
                application->commands.insert(Command::ShowDepth);
            {
                static bool render = false;
                if (ImGui::Checkbox("Render Octree", &render))
                    application->commands.insert(Command::OctreeRender);
            }

            ImGui::EndMenu();
        }
    }

    void MenuBar::render()
    {
        ImGui::BeginMainMenuBar();

        file_button();
        view_button();

        ImGui::EndMainMenuBar();
    }
}// namespace Editor
