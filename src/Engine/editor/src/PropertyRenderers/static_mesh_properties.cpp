#include <Graphics/imgui.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <PropertyRenderers/special_renderers.hpp>

namespace Engine
{

    template<typename Type>
    static void render_buffers_array(const Vector<Pointer<Type>>& buffers, const char* name)
    {
        Index index = 0;

        if (ImGui::CollapsingHeader(name))
        {
            ImGui::Indent(10);
            for (auto& buffer : buffers)
            {
                if (buffer)
                {
                    ImGui::Text("%s [%zu]: %zu", name, index, buffer.ptr()->buffer.size());
                }
                else
                {
                    ImGui::Text("%s [%zu]: None", name, index);
                }

                ++index;
            }

            if (buffers.empty())
            {
                ImGui::Text("%s: None", name);
            }

            ImGui::Unindent(10);
        }
    }

    static void renderer(Object* object, Struct* self, bool editable)
    {
        StaticMesh* mesh = object->instance_cast<StaticMesh>();

        auto min = mesh->bounds.min();
        auto max = mesh->bounds.max();

        if (ImGui::InputFloat3("Min bounds", &min.x, "%.3f"))
        {
            mesh->bounds.min(min);
        }

        if (ImGui::InputFloat3("Max bounds", &max.x, "%.3f"))
        {
            mesh->bounds.max(max);
        }

        ImGui::Separator();

        Index lod_index = 0;
        for (auto& lod : mesh->lods)
        {
            if (ImGui::TreeNodeEx(&lod, ImGuiTreeNodeFlags_CollapsingHeader, "LOD [%zu]", lod_index))
            {
                ImGui::Indent(10);
                render_buffers_array(lod.positions, "editor/Positions"_localized);
                render_buffers_array(lod.tex_coords, "editor/TexCoords"_localized);
                render_buffers_array(lod.colors, "editor/Colors"_localized);
                render_buffers_array(lod.normals, "editor/Normals"_localized);
                render_buffers_array(lod.tangents, "editor/Tangents"_localized);
                render_buffers_array(lod.binormals, "editor/Binormals"_localized);

                ImGui::Text("editor/Indices: %zu"_localized, lod.indices ? lod.indices.ptr()->buffer.size() : 0);
                ImGui::Unindent(10);
            }

            ++lod_index;
        }
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(StaticMesh::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
