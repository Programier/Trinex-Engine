#include <Core/editor_config.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Graphics/imgui.hpp>
#include <PropertyRenderers/special_renderers.hpp>
#include <Widgets/properties_window.hpp>

namespace Engine
{
	static void renderer(class ImGuiObjectProperties* window, void* object, Refl::Struct* self, bool editable)
	{
		PrimitiveComponent* component = reinterpret_cast<PrimitiveComponent*>(object);

		ImGui::TableNextRow();

		if (component && window->collapsing_header(reinterpret_cast<const void*>(&renderer), "editor/Bounds"_localized))
		{
			AABB_3Df bounds = component->bounding_box();
			ImGui::Indent(Settings::ed_collapsing_indent);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Min");

			ImGui::TableSetColumnIndex(1);
			float item_width = ImGui::GetContentRegionAvail().x;

			ImGui::SetNextItemWidth(item_width);
			ImGui::InputFloat3("##MinValue", const_cast<float*>(&bounds.min().x), "%.3f", ImGuiInputTextFlags_ReadOnly);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Max");

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(item_width);
			ImGui::InputFloat3("##MaxValue", const_cast<float*>(&bounds.max().x), "%.3f", ImGuiInputTextFlags_ReadOnly);

			ImGui::Unindent(Settings::ed_collapsing_indent);
		}
	}

	static void initialize_special_class_properties_renderers()
	{
		special_class_properties_renderers[reinterpret_cast<Refl::Struct*>(PrimitiveComponent::static_class_instance())] =
				renderer;
	}

	static InitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
