#include <Core/string_convert.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/textured_object.hpp>
#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <resouces.hpp>
#include <right_panel.hpp>
#include <texture_view.hpp>

namespace Editor
{

    void RightPanel::render_position_object(Engine::Translate* object)
    {
        if (!object)
            return;

        if (ImGui::TreeNode("Position"))
        {
            auto pos = object->position();

            if (ImGui::DragFloat("X", &pos.x, 0.01))
                object->move(pos, false);

            if (ImGui::DragFloat("Y", &pos.y, 0.01))
                object->move(pos, false);

            if (ImGui::DragFloat("Z", &pos.z, 0.01))
                object->move(pos, false);

            ImGui::TreePop();
        }
    }


    void RightPanel::render_scale_object(Engine::Scale* object)
    {
        if (!object)
            return;

        if (ImGui::TreeNode("Scale"))
        {
            auto scale = object->scale();

            if (ImGui::DragFloat("X", &scale.x, 0.01))
                object->scale(scale, false);

            if (ImGui::DragFloat("Y", &scale.y, 0.01))
                object->scale(scale, false);

            if (ImGui::DragFloat("Z", &scale.z, 0.01))
                object->scale(scale, false);

            ImGui::TreePop();
        }
    }

    void RightPanel::render_rotation_object(Engine::Rotate* object)
    {
        if (!object)
            return;

        if (ImGui::TreeNode("Rotation"))
        {
            auto rotation = glm::degrees(object->euler_angles());

            if (ImGui::DragFloat("X", &rotation.x, 0.01))
                object->rotate(glm::radians(rotation), false);

            if (ImGui::DragFloat("Y", &rotation.y, 0.01))
                object->rotate(glm::radians(rotation), false);

            if (ImGui::DragFloat("Z", &rotation.z, 0.01))
                object->rotate(glm::radians(rotation), false);

            ImGui::TreePop();
        }
    }

    void RightPanel::render_transform_object(Engine::ModelMatrix* object)
    {
        if (!object)
            return;

        if (ImGui::TreeNode("Transform"))
        {
            render_position_object(dynamic_cast<Engine::Translate*>(object));
            render_scale_object(dynamic_cast<Engine::Scale*>(object));
            render_rotation_object(dynamic_cast<Engine::Rotate*>(object));
            ImGui::TreePop();
        }
    }

    void RightPanel::render_textured_object(Engine::TexturedObject* object)
    {
        if (!object)
            return;

        if (ImGui::TreeNode("Textured Object"))
        {
            if (object->diffuse_texture().has_object() && ImGui::Button("Show Diffuse Texture"))
            {
                new TextureView(object->diffuse_texture(), this, "Texture View");
            }
            ImGui::TreePop();
        }
    }

    void RightPanel::render_drawable_object(Engine::Drawable* object)
    {
        if (!object)
            return;

        if (ImGui::TreeNode("Drawable"))
        {
            bool visible = object->visible();

            if (ImGui::Checkbox("Visible", &visible))
                object->visible(visible);

            ImGui::Text("Name: %s", Engine::Strings::to_string(object->name()).c_str());
            auto aabb = object->aabb();
            auto center = aabb.center();
            auto half_sizes = aabb.half_size();

            auto orig_aabb = object->original_aabb();
            auto orig_center = orig_aabb.center();
            auto orig_half_sizes = orig_aabb.half_size();


            ImGui::Text("Is Empty layer: %s", object->is_empty_layer() ? "True" : "False");
            ImGui::Text("Sub Nodes: %zu", object->sub_objects().size());

            if (ImGui::TreeNode("AABB"))
            {
                ImGui::DragFloat3("Center", glm::value_ptr(center), 0.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoInput);
                ImGui::DragFloat3("Half Size", glm::value_ptr(half_sizes), 0.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoInput);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Orig AABB"))
            {
                ImGui::DragFloat3("Center", glm::value_ptr(orig_center), 0.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoInput);
                ImGui::DragFloat3("Half Size", glm::value_ptr(orig_half_sizes), 0.f, 0.f, 0.f, "%.3f",
                                  ImGuiSliderFlags_NoInput);
                ImGui::TreePop();
            }

            if (ImGui::Button("Set as Render Root"))
            {
                Resources::object_for_rendering = object;
            }

            if (ImGui::TreeNode("Bones Map"))
            {
                auto& bones_map = object->bones_map();
                for (auto& ell : bones_map)
                {
                    if (ImGui::TreeNode(ell.first.c_str()))
                    {
                        ImGui::Text("ID: %zu", ell.second.ID);
                        for (int i = 0; i < 4; i++)
                        {
                            float* data = const_cast<float*>(glm::value_ptr(ell.second.offset[i]));
                            ImGui::DragFloat4(" ", data, 0.f, 0.f, 0.f, "%.3f",
                                              ImGuiSliderFlags_NoInput | ImGuiSliderFlags_ClampOnInput);
                        }

                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Animations"))
            {
                for (auto& animation : object->animation())
                {
                    if (ImGui::TreeNode(animation->name().c_str()))
                    {
                        ImGui::Text("Duration: %f\n", animation->duration());
                        ImGui::Text("Ticks: %lf\n", animation->ticks_per_second());
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }

            render_textured_object(dynamic_cast<Engine::TexturedObject*>(object));

            ImGui::TreePop();
        }
    }

    void RightPanel::render()
    {
        for (auto panel : std::list<Panel*>(_M_windows.begin(), _M_windows.end()))
        {
            panel->render();
        }

        ImGui::BeginChild("Parameters", {0, 0}, true);
        render_transform_object(dynamic_cast<Engine::ModelMatrix*>(Resources::object_for_properties));
        render_drawable_object(dynamic_cast<Engine::DrawableObject*>(Resources::object_for_properties));

        ImGui::EndChild();
    }
}// namespace Editor
