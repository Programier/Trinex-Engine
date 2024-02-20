#include <Engine/ActorComponents/scene_component.hpp>
#include <Graphics/imgui.hpp>
#include <PropertyRenderers/special_renderers.hpp>

namespace Engine
{
    static void renderer(Object* object, Struct* self, bool editable)
    {
        SceneComponent* component = object->instance_cast<SceneComponent>();

        if (ImGui::CollapsingHeader("editor/Transform"_localized))
        {
            bool changed = ImGui::InputFloat3("Location", &component->transform.location.x);
            changed      = ImGui::InputFloat3("Rotation", &component->transform.rotation.x) || changed;
            changed      = ImGui::InputFloat3("Scale", &component->transform.scale.x) || changed;

            if(changed)
            {
                component->on_transform_changed();
            }
        }
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(SceneComponent::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
