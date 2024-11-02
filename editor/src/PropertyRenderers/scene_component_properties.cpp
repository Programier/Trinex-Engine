#include <Engine/ActorComponents/scene_component.hpp>
#include <Graphics/imgui.hpp>
#include <PropertyRenderers/special_renderers.hpp>

namespace Engine
{
	static void renderer(class ImGuiObjectProperties* window, void* object, Refl::Struct* self, bool editable)
	{
		//        SceneComponent* component  = reinterpret_cast<SceneComponent*>(object);
		//        const Transform& transform = component->local_transform();
		//        Vector3D location          = transform.location();
		//        Vector3D rotation          = transform.rotation();
		//        Vector3D scale             = transform.scale();

		//        bool changed = ImGui::InputFloat3("Location", &location.x);
		//        changed      = ImGui::InputFloat3("Rotation", &rotation.x) || changed;
		//        changed      = ImGui::InputFloat3("Scale", &scale.x) || changed;

		//        if (changed)
		//        {
		//            component->location(location);
		//            component->rotation(rotation);
		//            component->scale(scale);
		//            component->on_transform_changed();
		//        }
	}

	static void initialize_special_class_properties_renderers()
	{
		//special_class_properties_renderers[reinterpret_cast<Struct*>(SceneComponent::static_class_instance())] = renderer;
	}

	static InitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
