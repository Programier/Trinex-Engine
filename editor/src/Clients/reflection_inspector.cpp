#include <Clients/imgui_client.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/scoped_type.hpp>
#include <Graphics/imgui.hpp>
#include <imgui_internal.h>

namespace Engine
{
	class ReflInspector : public ImGuiViewportClient
	{
		declare_class(ReflInspector, ImGuiViewportClient);

	public:
		static bool draw_object_info(Refl::Object* object)
		{
			bool open = false;

			ImGui::TableNextRow();

			if (object->is_a<Refl::ScopedType>() || object->is_a<Refl::Enum>())
			{
				ImGui::TableSetColumnIndex(0);
				open = ImGui::TreeNodeEx(object->name().c_str(), ImGuiTreeNodeFlags_SpanAllColumns);
			}
			else
			{
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", object->name().c_str());
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", object->refl_class_info()->class_name.c_str());

			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%p", object);

			return open;
		}


		void draw_tree(Refl::Object* object)
		{
			if (Refl::ScopedType* scope = Refl::Object::instance_cast<Refl::ScopedType>(object))
			{
				if (draw_object_info(object))
				{
					for (auto& [name, child] : scope->childs())
					{
						draw_tree(child);
					}

					ImGui::TreePop();
				}
			}
			else if (Refl::Enum* enum_instance = Refl::Object::instance_cast<Refl::Enum>(object))
			{
				if (draw_object_info(object))
				{
					for (auto& entry : enum_instance->entries())
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%s", entry.name.c_str());

						ImGui::TableSetColumnIndex(1);
						ImGui::Text("Enum Value");

						ImGui::TableSetColumnIndex(2);
						ImGui::Text("%d", entry.value);
					}

					ImGui::TreePop();
				}
			}
			else if (object)
			{
				draw_object_info(object);
			}
		}

		ReflInspector& update(float dt) override
		{
			ImGui::Begin("View", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

			if (ImGui::BeginTable("TreeTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Class Name");
				ImGui::TableSetupColumn("Value");
				ImGui::TableHeadersRow();

				draw_tree(Refl::Object::static_root());
				ImGui::EndTable();
			}
			ImGui::End();

			return *this;
		}

		ReflInspector& build_dock(uint32_t dock) override
		{
			ImGui::DockBuilderDockWindow("View", dock);
			return *this;
		}
	};

	implement_engine_class_default_init(ReflInspector, 0);
}// namespace Engine
