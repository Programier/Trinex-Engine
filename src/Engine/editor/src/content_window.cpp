#include <Core/class.hpp>
#include <Core/logger.hpp>
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
        if (ImGui::Button("editor/Create new asset"_localized))
        {
            ImGuiRenderer::Window::current()->window_list.create<ImGuiCreateNewAsset>(package);
            return false;
        }

        if (_M_selected && ImGui::Button("editor/Reload"_localized))
        {
            _M_selected->reload();
            return false;
        }

        bool is_editable_object = _M_selected && !_M_selected->is_internal();

        if (is_editable_object && ImGui::Button("editor/Rename"_localized))
        {
            ImGuiRenderer::Window::current()->window_list.create<ImGuiRenameObject>(_M_selected);
            return false;
        }

        if (is_editable_object && ImGui::Button("editor/Delete"_localized))
        {
            Package* package = _M_selected->package();
            package->remove_object(_M_selected);
            delete _M_selected;
            _M_selected = nullptr;
            on_object_selected(nullptr);
            return false;
        }
        return true;
    }

    bool ImGuiContentBrowser::render(RenderViewport* viewport)
    {
        static const ImVec2 item_size = {100, 100};

        ImGui::Begin(name());
        if (package == nullptr)
        {
            ImGui::Text("%s!", "editor/No package selected"_localized);
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

        float padding = ImGui::GetStyle().FramePadding.x;

        for (auto& [name, object] : package->objects())
        {
            if (not_first_item)
            {
                ImGui::SameLine();

                if (ImGui::GetCursorPosX() + item_size.x >= content_size.x)
                {
                    ImGui::NewLine();
                    ImGui::NewLine();
                }
            }
            else
            {
                not_first_item = true;
            }

            ImGui::BeginGroup();

            ImGuiRenderer::ImGuiTexture* imgui_texture = Icons::find_imgui_icon(object);

            if (imgui_texture && imgui_texture->handle())
            {
                ImGui::PushID(name.to_string().c_str());

                bool selected = _M_selected == object;

                if (selected)
                {
                    static ImVec4 color1 = ImVec4(79.f / 255.f, 109.f / 255.f, 231.f / 255.f, 1.0),
                                  color2 = ImVec4(114.f / 255.f, 138.f / 255.f, 233.f / 255.f, 1.0);

                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(color1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(color2));
                }

                ImVec2 item_start = ImGui::GetCursorPos();

                if (ImGui::ImageButton(imgui_texture->handle(), item_size))
                {
                    _M_selected = object;
                    on_object_selected(object);
                }

                if (selected)
                {
                    ImGui::PopStyleColor(2);
                }


                ImVec2 current_pos = ImGui::GetCursorPos();

                if (imgui_texture->texture() == Icons::default_texture())
                {
                    const char* class_name = object->class_instance()->base_name_splitted().c_str();
                    ImVec2 text_size       = ImGui::CalcTextSize(class_name, nullptr, false, item_size.x);

                    ImVec2 text_pos = item_start + ImVec2(((item_size.x / 2) - (text_size.x / 2)) + padding,
                                                          (item_size.y / 2) - (text_size.y / 2));

                    ImGui::SetCursorPos(text_pos);
                    ImGui::PushTextWrapPos(text_pos.x + item_size.x);

                    ImGui::TextWrapped("%s", class_name);
                    ImGui::PopTextWrapPos();
                }


                String object_name = Strings::make_sentence(name);
                float offset = (item_size.x - ImGui::CalcTextSize(object_name.c_str(), nullptr, false, item_size.x).x) / 2.f;
                current_pos.x += offset;
                ImGui::SetCursorPos(current_pos);

                ImGui::PushTextWrapPos(current_pos.x + item_size.x - offset);
                ImGui::TextWrapped("%s", object_name.c_str());
                ImGui::PopTextWrapPos();

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
        return "editor/Content Browser Title"_localized;
    }
}// namespace Engine
