#include <dock_window.hpp>
#include <imgui.h>

namespace Engine
{
    void make_dock_window(const char* name, void (*callback)(), unsigned int flags)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);


        ImGuiWindowFlags dock_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                                      ImGuiWindowFlags_NoBringToFrontOnFocus | flags;

        if (ImGui::Begin(name, nullptr, dock_flags))
        {
            auto id                            = ImGui::GetID(name);
            ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGui::DockSpace(id, ImVec2(0.0f, 0.0f), dockspace_flags);

            if (callback)
                callback();
        }

        ImGui::End();
    }
}// namespace Engine
