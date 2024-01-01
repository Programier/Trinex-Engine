#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <icons.hpp>
#include <imgui_internal.h>
#include <imgui_windows.hpp>


namespace Engine
{
    bool ImGuiContentBrowser::show_context_menu(void* userdata)
    {
        if (ImGui::Button("Create new asset"))
        {
            ImGuiRenderer::Window::current()->window_list.create<ImGuiCreateNewAsset>(package);
            return false;
        }

        if (_M_selected && ImGui::Button("Reload"))
        {
            _M_selected->reload();
        }
        return true;
    }

    bool ImGuiContentBrowser::render(RenderViewport* viewport)
    {
        static const ImVec2 item_size = {100, 100};

        ImGui::Begin(name());
        if (package == nullptr)
        {
            ImGui::Text("No package selected!");
            ImGui::End();
            return true;
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            _M_show_context_menu = true;
        }

        if (_M_show_context_menu)
        {
            ImGui::OpenPopup("##NoName1");
            _M_show_context_menu =
                    ImGuiRenderer::BeginPopup("##NoName1", 0, &ImGuiContentBrowser::show_context_menu, this, viewport);
        }

        ImVec2 content_size = ImGui::GetContentRegionAvail();
        bool not_first_item = false;

        for (auto& [name, object] : package->objects())
        {
            if (not_first_item)
            {
                ImGui::SameLine();
                if (ImGui::GetCursorPosX() + item_size.x > content_size.x)
                {
                    ImGui::NewLine();
                    not_first_item = false;
                }
            }
            else
            {
                not_first_item = true;
            }

            ImGui::BeginGroup();

            void* imgui_texture = find_imgui_icon(object);

            if (imgui_texture)
            {
                ImGui::PushID(name.to_string().c_str());
                bool selected = _M_selected == object;
                if (selected)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                }

                if (ImGui::ImageButton(imgui_texture, item_size))
                {
                    _M_selected = object;
                    on_object_selected(object);
                }

                if (selected)
                {
                    ImGui::PopStyleColor();
                }

                ImGui::TextWrapped("%s", name.c_str());

                ImGui::PopID();
            }
            else
            {
                if (ImGui::Selectable(name.c_str(), _M_selected == object, 0, item_size))
                {
                    _M_selected = object;
                    on_object_selected(object);
                }
            }

            ImGui::EndGroup();
        }

        ImGui::End();

        return true;
    }

    const char* ImGuiContentBrowser::name()
    {
        return "Content Browser";
    }
}// namespace Engine
