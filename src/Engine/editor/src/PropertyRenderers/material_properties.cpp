#include <Core/engine_loading_controllers.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <PropertyRenderers/special_renderers.hpp>
#include <Widgets/properties_window.hpp>
#include <editor_config.hpp>


namespace Engine
{
    static TreeMap<MaterialParameterType, void (*)(MaterialParameter*)> m_parameter_renderers;

#define declare_parameter_renderer(type)                                                                                         \
    {                                                                                                                            \
        MaterialParameterType::type, trinex_editor_##type##_renderer                                                             \
    }


#define param_renderer_func(type) static void trinex_editor_##type##_renderer(MaterialParameter* parameter)

    param_renderer_func(Bool)
    {
        BoolMaterialParameter* boolean = reinterpret_cast<BoolMaterialParameter*>(parameter);
        ImGui::Checkbox("##Val", &boolean->param);
    }

    param_renderer_func(Int)
    {
        IntMaterialParameter* integer = reinterpret_cast<IntMaterialParameter*>(parameter);
        ImGui::DragInt("##Val", &integer->param);
    }

    param_renderer_func(UInt)
    {
        UIntMaterialParameter* integer = reinterpret_cast<UIntMaterialParameter*>(parameter);
        ImGui::DragScalar("##Val", ImGuiDataType_U32, &integer->param);
    }

    param_renderer_func(Float)
    {
        UIntMaterialParameter* integer = reinterpret_cast<UIntMaterialParameter*>(parameter);
        ImGui::DragScalar("##Val", ImGuiDataType_Float, &integer->param);
    }

    param_renderer_func(BVec2)
    {
        BVec2MaterialParameter* vec = reinterpret_cast<BVec2MaterialParameter*>(parameter);
        ImGui::Checkbox("##1", &vec->param.r);
        ImGui::SameLine();
        ImGui::Checkbox("##2", &vec->param.g);
    }

    param_renderer_func(BVec3)
    {
        BVec3MaterialParameter* vec = reinterpret_cast<BVec3MaterialParameter*>(parameter);
        ImGui::Checkbox("##1", &vec->param.r);
        ImGui::SameLine();
        ImGui::Checkbox("##2", &vec->param.g);
        ImGui::SameLine();
        ImGui::Checkbox("##3", &vec->param.b);
    }

    param_renderer_func(BVec4)
    {
        BVec4MaterialParameter* vec = reinterpret_cast<BVec4MaterialParameter*>(parameter);
        ImGui::Checkbox("##1", &vec->param.r);
        ImGui::SameLine();
        ImGui::Checkbox("##2", &vec->param.g);
        ImGui::SameLine();
        ImGui::Checkbox("##3", &vec->param.b);
        ImGui::SameLine();
        ImGui::Checkbox("##4", &vec->param.a);
    }

    param_renderer_func(IVec2)
    {
        IVec2MaterialParameter* vec = reinterpret_cast<IVec2MaterialParameter*>(parameter);
        ImGui::DragInt2("##Val", &vec->param.r);
    }


    param_renderer_func(IVec3)
    {
        IVec3MaterialParameter* vec = reinterpret_cast<IVec3MaterialParameter*>(parameter);
        ImGui::DragInt3("##Val", &vec->param.r);
    }

    param_renderer_func(IVec4)
    {
        IVec4MaterialParameter* vec = reinterpret_cast<IVec4MaterialParameter*>(parameter);
        ImGui::DragInt4("##Val", &vec->param.r);
    }

    param_renderer_func(UVec2)
    {
        UVec2MaterialParameter* vec = reinterpret_cast<UVec2MaterialParameter*>(parameter);
        ImGui::DragScalarN("##Val", ImGuiDataType_U32, &vec->param.r, 2);
    }

    param_renderer_func(UVec3)
    {
        UVec3MaterialParameter* vec = reinterpret_cast<UVec3MaterialParameter*>(parameter);
        ImGui::DragScalarN("##Val", ImGuiDataType_U32, &vec->param.r, 3);
    }

    param_renderer_func(UVec4)
    {
        UVec4MaterialParameter* vec = reinterpret_cast<UVec4MaterialParameter*>(parameter);
        ImGui::DragScalarN("##Val", ImGuiDataType_U32, &vec->param.r, 4);
    }

    param_renderer_func(Vec2)
    {
        Vec2MaterialParameter* vec = reinterpret_cast<Vec2MaterialParameter*>(parameter);
        ImGui::DragScalarN("##Val", ImGuiDataType_Float, &vec->param.r, 2);
    }

    param_renderer_func(Vec3)
    {
        Vec3MaterialParameter* vec = reinterpret_cast<Vec3MaterialParameter*>(parameter);
        ImGui::DragScalarN("##Val", ImGuiDataType_Float, &vec->param.r, 3);
    }

    param_renderer_func(Vec4)
    {
        Vec4MaterialParameter* vec = reinterpret_cast<Vec4MaterialParameter*>(parameter);
        ImGui::DragScalarN("##Val", ImGuiDataType_Float, &vec->param.r, 4);
    }

    param_renderer_func(Mat3)
    {}

    param_renderer_func(Mat4)
    {
        Mat4MaterialParameter* mat = reinterpret_cast<Mat4MaterialParameter*>(parameter);
        ImGui::Checkbox("editor/Is Model Matrix"_localized, &mat->bind_model_matrix);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::DragScalarN("##Val1", ImGuiDataType_Float, &mat->param[0], 4);
        ImGui::DragScalarN("##Val2", ImGuiDataType_Float, &mat->param[1], 4);
        ImGui::DragScalarN("##Val3", ImGuiDataType_Float, &mat->param[2], 4);
        ImGui::DragScalarN("##Val4", ImGuiDataType_Float, &mat->param[3], 4);
        ImGui::PopItemWidth();
    }

    param_renderer_func(Sampler)
    {}

    param_renderer_func(CombinedImageSampler2D)
    {
        m_parameter_renderers[MaterialParameterType::Texture2D](parameter);
    }

    param_renderer_func(Texture2D)
    {
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
    }

    static void on_init()
    {
        m_parameter_renderers = {declare_parameter_renderer(Bool),     declare_parameter_renderer(Int),
                                 declare_parameter_renderer(UInt),     declare_parameter_renderer(Float),
                                 declare_parameter_renderer(BVec2),    declare_parameter_renderer(BVec3),
                                 declare_parameter_renderer(BVec4),    declare_parameter_renderer(IVec2),
                                 declare_parameter_renderer(IVec3),    declare_parameter_renderer(IVec4),
                                 declare_parameter_renderer(UVec2),    declare_parameter_renderer(UVec3),
                                 declare_parameter_renderer(UVec4),    declare_parameter_renderer(Vec2),
                                 declare_parameter_renderer(Vec3),     declare_parameter_renderer(Vec4),
                                 declare_parameter_renderer(Mat3),     declare_parameter_renderer(Mat4),
                                 declare_parameter_renderer(Sampler),  declare_parameter_renderer(CombinedImageSampler2D),
                                 declare_parameter_renderer(Texture2D)};
    }

    static PreInitializeController initializer(on_init);

    struct MaterialPropertiesUserData : public UserData::Entry {
        uint definition_index = 0;
    };

    static void renderer(class ImGuiObjectProperties* window, void* object, Struct* self, bool editable)
    {
        Material* material = reinterpret_cast<Material*>(object);

        if (material == nullptr)
        {
            return;
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (window->collapsing_header("editor/Parameters"_localized))
        {
            ImGui::Indent(editor_config.collapsing_indent);

            for (auto& param : material->material()->parameters())
            {
                ImGui::PushID(param.second);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", param.first.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

                auto renderer = m_parameter_renderers[param.second->type()];
                if (renderer)
                {
                    renderer(param.second);
                }
                ImGui::PopID();
            }
            ImGui::Unindent(editor_config.collapsing_indent);
        }
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(Material::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
