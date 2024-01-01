#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <imgui_windows.hpp>


namespace Engine
{
    bool ImGuiContentBrowser::show_context_menu(void* userdata)
    {
        RenderViewport* viewport = reinterpret_cast<RenderViewport*>(userdata);
        if (ImGui::Button("Create new asset"))
        {
            ImGuiRenderer::Window::current()->window_list.create<ImGuiCreateNewAsset>(viewport, package);
            return false;
        }
        return true;
    }

    bool ImGuiContentBrowser::render(RenderViewport* viewport)
    {
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


        for (auto& [name, object] : package->objects())
        {
            if(ImGui::Selectable(object->string_name().c_str(), _M_selected == object))
            {
                _M_selected = object;
                on_object_selected.trigger(object);
            }
        }

        ImGui::End();

        return true;
    }

    const char* ImGuiContentBrowser::name()
    {
        return "Content Browser";
    }
}// namespace Engine
