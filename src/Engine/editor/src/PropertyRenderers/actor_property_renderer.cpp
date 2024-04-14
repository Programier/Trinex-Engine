#include <Engine/ActorComponents/actor_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Graphics/imgui.hpp>
#include <PropertyRenderers/special_renderers.hpp>


namespace Engine
{
    static void renderer(class ImGuiObjectProperties* window, void* object, Struct* self, bool editable)
    {
//        Actor* actor = reinterpret_cast<Actor*>(object);

//        if (ImGui::CollapsingHeader("editor/Components"_localized))
//        {
//            ImGui::Indent(5.f);
//            for (ActorComponent* component : actor->owned_components())
//            {
//                ImGui::PushID(component);
//                if (ImGui::CollapsingHeader(component->string_name().c_str()))
//                {
//                    ImGui::Indent(5.f);
//                    render_struct_properties(window, component, reinterpret_cast<Struct*>(component->class_instance()), editable);
//                    ImGui::Unindent(5.f);
//                }
//                ImGui::PopID();
//            }
//            ImGui::Unindent(5.f);
//        }
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(Actor::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
