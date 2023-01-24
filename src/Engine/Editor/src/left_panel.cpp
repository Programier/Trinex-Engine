#include <Core/string_convert.hpp>
#include <Core/string_format.hpp>
#include <ImGui/imgui.h>
#include <editor.hpp>
#include <left_panel.hpp>
#include <resouces.hpp>

namespace Editor
{
    static int node = 1;

    static void draw_scene_list(Engine::DrawableObject* layer)
    {
        if (!layer)
            return;

        const auto flags = (layer == dynamic_cast<Engine::DrawableObject*>(Resources::object_for_properties)
                                    ? ImGuiTreeNodeFlags_Selected
                                    : ImGuiTreeNodeFlags_None) |
                           ImGuiTreeNodeFlags_OpenOnArrow;

        auto state = ImGui::TreeNodeEx(Engine::Strings::format("Node_{}", node).c_str(), flags, "%s",
                                       Engine::Strings::to_string(layer->name()).c_str());

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            Resources::object_for_properties = layer;
            application->commands.insert(Command::ObjectChanged);
        }

        if (state)
        {
            for (auto ell : layer->sub_objects())
            {
                node++;
                draw_scene_list(ell);
            }
            ImGui::TreePop();
        }
    }


    void LeftPanel::process_keyboard()
    {
        auto current_key = Engine::KeyboardEvent::just_pressed();

        switch (current_key)
        {
            case Engine::KEY_DELETE:
            {
                Engine::DrawableObject* object = dynamic_cast<Engine::DrawableObject*>(Resources::object_for_properties);

                if (object)
                {
                    Engine::DrawableObject* parent = object->parent();
                    if (parent)
                    {
                        parent->remove_object(object);
                        delete object;
                        Resources::object_for_properties = parent;
                    }
                }

                break;
            }
            default:
                break;
        }
    }

    void LeftPanel::render()
    {
        ImGui::BeginChild("Scene", {0, 0}, true, ImGuiWindowFlags_HorizontalScrollbar);
        process_keyboard();
        draw_scene_list(&Editor::Resources::scene);
        node = 1;
        ImGui::EndChild();
    }
}// namespace Editor
