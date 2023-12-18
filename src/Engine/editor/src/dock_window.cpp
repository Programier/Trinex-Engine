#include <dock_window.hpp>
#include <imgui.h>

namespace Engine
{
    void make_dock_window()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);


        static ImGuiWindowFlags dock_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse |
                                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                                             ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("EditorDockWindow", nullptr, dock_flags))
        {
            auto id                            = ImGui::GetID("EditorDockWindow##Dock");
            ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGui::DockSpace(id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGui::End();
    }
}// namespace Engine
