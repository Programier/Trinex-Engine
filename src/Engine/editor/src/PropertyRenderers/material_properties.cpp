#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <PropertyRenderers/special_renderers.hpp>
#include <Widgets/imgui_windows.hpp>
#include <editor_config.hpp>


namespace Engine
{

    static Array<void (*)(MaterialParameter*), static_cast<size_t>(MaterialParameterType::__COUNT__)> m_parameter_renderers = {
            [](MaterialParameter* parameter) {
                BoolMaterialParameter* boolean = reinterpret_cast<BoolMaterialParameter*>(parameter);
                ImGui::Checkbox("##Val", &boolean->param);
            },
            [](MaterialParameter* parameter) {
                IntMaterialParameter* integer = reinterpret_cast<IntMaterialParameter*>(parameter);
                ImGui::DragInt("##Val", &integer->param);
            },
            [](MaterialParameter* parameter) {
                UIntMaterialParameter* integer = reinterpret_cast<UIntMaterialParameter*>(parameter);
                ImGui::DragScalar("##Val", ImGuiDataType_U32, &integer->param);
            },
            [](MaterialParameter* parameter) {
                UIntMaterialParameter* integer = reinterpret_cast<UIntMaterialParameter*>(parameter);
                ImGui::DragScalar("##Val", ImGuiDataType_Float, &integer->param);
            },
            [](MaterialParameter* parameter) {
                BVec2MaterialParameter* vec = reinterpret_cast<BVec2MaterialParameter*>(parameter);
                ImGui::Checkbox("##1", &vec->param.r);
                ImGui::SameLine();
                ImGui::Checkbox("##2", &vec->param.g);
            },
            [](MaterialParameter* parameter) {
                BVec3MaterialParameter* vec = reinterpret_cast<BVec3MaterialParameter*>(parameter);
                ImGui::Checkbox("##1", &vec->param.r);
                ImGui::SameLine();
                ImGui::Checkbox("##2", &vec->param.g);
                ImGui::SameLine();
                ImGui::Checkbox("##3", &vec->param.b);
            },
            [](MaterialParameter* parameter) {
                BVec4MaterialParameter* vec = reinterpret_cast<BVec4MaterialParameter*>(parameter);
                ImGui::Checkbox("##1", &vec->param.r);
                ImGui::SameLine();
                ImGui::Checkbox("##2", &vec->param.g);
                ImGui::SameLine();
                ImGui::Checkbox("##3", &vec->param.b);
                ImGui::SameLine();
                ImGui::Checkbox("##4", &vec->param.a);
            },
            [](MaterialParameter* parameter) {
                IVec2MaterialParameter* vec = reinterpret_cast<IVec2MaterialParameter*>(parameter);
                ImGui::DragInt2("##Val", &vec->param.r);
            },
            [](MaterialParameter* parameter) {
                IVec3MaterialParameter* vec = reinterpret_cast<IVec3MaterialParameter*>(parameter);
                ImGui::DragInt3("##Val", &vec->param.r);
            },
            [](MaterialParameter* parameter) {
                IVec4MaterialParameter* vec = reinterpret_cast<IVec4MaterialParameter*>(parameter);
                ImGui::DragInt4("##Val", &vec->param.r);
            },
            [](MaterialParameter* parameter) {
                UVec2MaterialParameter* vec = reinterpret_cast<UVec2MaterialParameter*>(parameter);
                ImGui::DragScalarN("##Val", ImGuiDataType_U32, &vec->param.r, 2);
            },
            [](MaterialParameter* parameter) {
                UVec3MaterialParameter* vec = reinterpret_cast<UVec3MaterialParameter*>(parameter);
                ImGui::DragScalarN("##Val", ImGuiDataType_U32, &vec->param.r, 3);
            },
            [](MaterialParameter* parameter) {
                UVec4MaterialParameter* vec = reinterpret_cast<UVec4MaterialParameter*>(parameter);
                ImGui::DragScalarN("##Val", ImGuiDataType_U32, &vec->param.r, 4);
            },
            [](MaterialParameter* parameter) {
                Vec2MaterialParameter* vec = reinterpret_cast<Vec2MaterialParameter*>(parameter);
                ImGui::DragScalarN("##Val", ImGuiDataType_Float, &vec->param.r, 2, 1.0);
            },
            [](MaterialParameter* parameter) {
                Vec3MaterialParameter* vec = reinterpret_cast<Vec3MaterialParameter*>(parameter);
                ImGui::DragScalarN("##Val", ImGuiDataType_Float, &vec->param.r, 3);
            },
            [](MaterialParameter* parameter) {
                Vec4MaterialParameter* vec = reinterpret_cast<Vec4MaterialParameter*>(parameter);
                ImGui::DragScalarN("##Val", ImGuiDataType_Float, &vec->param.r, 4);
            },
            [](MaterialParameter* parameter) {
                // Mat3
            },
            [](MaterialParameter* parameter) {
                // Mat4
            },
            [](MaterialParameter* parameter) {
                // Sampler
            },
            [](MaterialParameter* parameter) {
                m_parameter_renderers[static_cast<size_t>(MaterialParameterType::Texture2D)](parameter);
            },
            [](MaterialParameter* parameter) {
                ImGui::PushID(parameter);
                Texture2D* texture =
                        reinterpret_cast<Texture2D*>(reinterpret_cast<BindingMaterialParameter*>(parameter)->texture_param());
                ImGui::Image(texture, ImVec2(100, 100));

                if (ImGui::BeginDragDropTarget())
                {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
                    if (payload)
                    {
                        IM_ASSERT(payload->DataSize == sizeof(Object*));
                        Object* new_object = *reinterpret_cast<Object**>(payload->Data);
                        if (Texture2D* new_texture = Object::instance_cast<Texture2D>(new_object))
                        {
                            reinterpret_cast<BindingMaterialParameter*>(parameter)->texture_param(new_texture);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
            },
            [](MaterialParameter* parameter) {
                // ModelMatrix
            },
    };


    struct MaterialPropertiesUserData : public UserData::Entry {
        uint definition_index = 0;
    };

    static void renderer(class ImGuiObjectProperties* window, void* object, Struct* self, bool editable)
    {
        Material* material = reinterpret_cast<Material*>(object);

        static Name userdata_name = "TrinexEngine::Editor::MaterialPropertiesRenderer::definition_index";
        uint& m_definition_index =
                window->userdata.find_or_create_userdata<MaterialPropertiesUserData>(userdata_name)->definition_index;

        if (material == nullptr)
        {
            return;
        }

        const ImVec2 window_size      = ImGui::GetContentRegionAvail();
        const ImVec2 window_half_size = window_size / 2.f;

        if (ImGui::CollapsingHeader("editor/Definitions"_localized))
        {
            ImGui::Indent(editor_config.collapsing_indent);
            auto& definitions = material->compile_definitions;

            static auto update_index = [](decltype(definitions)& definitions, uint_t& index) {
                index = glm::min<uint_t>(index, definitions.size() - 1);
            };

            update_index(definitions, m_definition_index);

            {
                const float button_size_x = ImGui::GetFrameHeight();
                const ImVec2 button_size  = ImVec2(button_size_x, button_size_x);

                ImGui::SetNextItemWidth(window_size.x - (button_size_x + ImGui::GetStyle().ItemSpacing.x) * 2);
                if (ImGui::InputScalar("##Index", ImGuiDataType_U32, &m_definition_index))
                {
                    update_index(definitions, m_definition_index);
                }

                ImGui::SameLine();

                if (ImGui::Button("+", button_size))
                {
                    definitions.emplace(definitions.begin() + m_definition_index);
                }

                ImGui::SameLine();

                if (ImGui::Button("-", button_size))
                {
                    definitions.erase(definitions.begin() + m_definition_index);
                    update_index(definitions, m_definition_index);
                }
            }

            for (auto& definition : definitions)
            {
                ImGui::PushID(&definition);
                ImGui::SetNextItemWidth(window_half_size.x);
                ImGuiRenderer::InputText("##Key"_localized, definition.key);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(window_half_size.x);
                ImGuiRenderer::InputText("##Value"_localized, definition.value);
                ImGui::PopID();
            }

            ImGui::Unindent(editor_config.collapsing_indent);
        }

        if (ImGui::CollapsingHeader("editor/Parameters"_localized))
        {
            ImGui::Indent(editor_config.collapsing_indent);
            ImGui::BeginTable("##Params", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner);

            for (auto& param : material->material()->parameters())
            {
                ImGui::PushID(param.second);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", param.first.c_str());
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

                auto renderer = m_parameter_renderers[static_cast<size_t>(param.second->type())];
                if (renderer)
                {
                    renderer(param.second);
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
            ImGui::Unindent(editor_config.collapsing_indent);
        }


        //    ImGui::SeparatorText("Pipeline");
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(Material::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
