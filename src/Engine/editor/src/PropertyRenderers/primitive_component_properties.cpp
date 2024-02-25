#include <Engine/ActorComponents/primitive_component.hpp>
#include <Graphics/imgui.hpp>
#include <PropertyRenderers/special_renderers.hpp>

namespace Engine
{
    static void renderer(Object* object, Struct* self, bool editable)
    {
        PrimitiveComponent* component = object->instance_cast<PrimitiveComponent>();

        if (component && ImGui::CollapsingHeader("editor/Bounds"_localized))
        {
            AABB_3Df bounds = component->bounding_box();

            ImGui::InputFloat3("Min", const_cast<float*>(&bounds.min().x), "%.3f", ImGuiInputTextFlags_ReadOnly);
            ImGui::InputFloat3("Max", const_cast<float*>(&bounds.max().x), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(PrimitiveComponent::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
