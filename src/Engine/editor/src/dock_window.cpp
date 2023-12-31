#include <dock_window.hpp>
#include <imgui.h>

namespace Engine
{
    void make_dock_window(const char* name, unsigned int flags, void (*callback)(void* userdata), void* userdata)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);


        ImGuiWindowFlags dock_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                                      ImGuiWindowFlags_NoBringToFrontOnFocus | flags;

        if (ImGui::Begin(name, nullptr, dock_flags))
        {
            if (callback)
                callback(userdata);
        }

        ImGui::End();
    }
}// namespace Engine
