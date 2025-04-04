#include <Clients/material/visual_material_client.hpp>
#include <Core/blueprints.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/group.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>
#include <Widgets/property_renderer.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	trinex_implement_class(Engine::VisualMaterialEditorClient, 0)
	{
		register_client(VisualMaterial::static_class_instance(), static_class_instance());
	}

	class ImGuiNodeProperties : public PropertyRenderer
	{
	public:
		const char* name() const override { return static_name(); }
		static const char* static_name() { return "editor/Node Properties"_localized; }
	};

	struct HorizontalLayout {
		HorizontalLayout() { ImGui::BeginHorizontal(this); }
		~HorizontalLayout() { ImGui::EndHorizontal(); }
	};

	struct VerticalLayout {
		VerticalLayout() { ImGui::BeginVertical(this); }
		~VerticalLayout() { ImGui::EndVertical(); }
	};

	static inline void render_default_vector1b(VisualMaterialGraph::Pin* pin)
	{
		ImGui::Checkbox("##vector1b", reinterpret_cast<bool*>(pin->default_value()->address()));
	}

	static void render_default_vector_nb(const char* label, bool* data, int components)
	{
		auto* window = ImGui::GetCurrentWindow();

		if (window->SkipItems)
			return;

		auto& style = ImGui::GetStyle();

		ImGui::BeginGroup();
		ImGui::PushID(label);
		ImGui::PushMultiItemsWidths(components, ImGui::CalcItemWidth());

		for (int i = 0; i < components; i++)
		{
			ImGui::PushID(i);
			if (i > 0)
				ImGui::SameLine(0, style.ItemInnerSpacing.x);

			ImGui::Checkbox("##vectornb", data);
			ImGui::PopID();
			ImGui::PopItemWidth();
			++data;
		}

		ImGui::PopID();
		ImGui::EndGroup();
	}

	static inline void render_default_vector2b(VisualMaterialGraph::Pin* pin)
	{
		render_default_vector_nb("##vector2b", reinterpret_cast<bool*>(pin->default_value()->address()), 2);
	}

	static inline void render_default_vector3b(VisualMaterialGraph::Pin* pin)
	{
		render_default_vector_nb("##vector3b", reinterpret_cast<bool*>(pin->default_value()->address()), 3);
	}

	static inline void render_default_vector4b(VisualMaterialGraph::Pin* pin)
	{
		render_default_vector_nb("##vector4b", reinterpret_cast<bool*>(pin->default_value()->address()), 4);
	}

	static void render_default_vector1i(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector1i", ImGuiDataType_S32, ptr, 1);
	}

	static void render_default_vector2i(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector2i", ImGuiDataType_S32, ptr, 2);
	}

	static void render_default_vector3i(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector3i", ImGuiDataType_S32, ptr, 3);
	}

	static void render_default_vector4i(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector4i", ImGuiDataType_S32, ptr, 4);
	}

	static void render_default_vector1u(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector1u", ImGuiDataType_U32, ptr, 1);
	}

	static void render_default_vector2u(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector2u", ImGuiDataType_U32, ptr, 2);
	}

	static void render_default_vector3u(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector3u", ImGuiDataType_U32, ptr, 3);
	}

	static void render_default_vector4u(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector4u", ImGuiDataType_U32, ptr, 4);
	}

	static void render_default_vector1f(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector1f", ImGuiDataType_Float, ptr, 1);
	}

	static void render_default_vector2f(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector2f", ImGuiDataType_Float, ptr, 2);
	}

	static void render_default_vector3f(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector3f", ImGuiDataType_Float, ptr, 3);
	}

	static void render_default_vector4f(VisualMaterialGraph::Pin* pin)
	{
		void* ptr = pin->default_value()->address();
		ImGui::InputScalarN("###vector4f", ImGuiDataType_Float, ptr, 4);
	}

	static void render_default_matrix3f(VisualMaterialGraph::Pin* pin)
	{
		ImGui::InputScalarN("", ImGuiDataType_Float, pin->default_value()->address(), 1);
	}

	static void render_default_matrix4f(VisualMaterialGraph::Pin* pin)
	{
		ImGui::InputScalarN("", ImGuiDataType_Float, pin->default_value()->address(), 1);
	}

	static void (*s_default_type_renderers[17])(VisualMaterialGraph::Pin* pin) = {nullptr};

	// static void render_default_matrix3f(VisualMaterialGraph::Pin* pin)
	// {
	// 	float* value = static_cast<float*>(pin->default_value());
	// 	for (int i = 0; i < 3; ++i)
	// 	{
	// 		ImGui::InputFloat3(("##matrix3f_row" + std::to_string(i)).c_str(), &value[i * 3]);
	// 	}
	// }

	// static void render_default_matrix4f(VisualMaterialGraph::Pin* pin)
	// {
	// 	float* value = static_cast<float*>(pin->default_value());
	// 	for (int i = 0; i < 4; ++i)
	// 	{
	// 		ImGui::InputFloat4(("##matrix4f_row" + std::to_string(i)).c_str(), &value[i * 4]);
	// 	}
	// }

	static void show_label(const char* label, ImColor color)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
		auto size = ImGui::CalcTextSize(label);

		auto padding = ImGui::GetStyle().FramePadding;
		auto spacing = ImGui::GetStyle().ItemSpacing;

		ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

		auto rectMin = ImGui::GetCursorScreenPos() - padding;
		auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

		auto drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
		ImGui::TextUnformatted(label);
	};


	static bool match_filter(Refl::Struct* self, const String& filter)
	{
		if (self->is_class())
		{
			if (!filter.empty())
			{
				const String& name = self->name().to_string();

				if (name.length() < filter.length())
					return false;

				const char* symbol = name.c_str();

				for (char ch : filter)
				{
					if (std::tolower(ch) != std::tolower(*symbol++))
						return false;
				}

				return true;
			}

			return true;
		}
		return false;
	}

	static bool match_filter(Group* group, const String& filter)
	{
		if (filter.empty())
			return true;

		for (Refl::Struct* instance : group->structs())
		{
			if (match_filter(instance, filter))
				return true;
		}

		for (auto* child : group->childs())
		{
			if (match_filter(child, filter))
				return true;
		}

		return false;
	}

	static Refl::Class* render_node_types(Group* group, const String& filter, bool changed)
	{
		if (!group)
			return nullptr;

		Refl::Class* current = nullptr;

		for (auto* child : group->childs())
		{
			ImGui::PushID(child->name().c_str());

			bool is_visible = false;

			if (changed)
			{
				is_visible = match_filter(child, filter);
				ImGui::GetStateStorage()->SetBool(ImGui::GetID("is_visible"), is_visible);
			}
			else
			{
				is_visible = ImGui::GetStateStorage()->GetBool(ImGui::GetID("is_visible"));
			}

			if (is_visible)
			{
				if (changed)
				{
					ImGui::SetNextItemOpen(!filter.empty());
				}

				if (ImGui::TreeNodeEx(child->name().c_str()))
				{
					ImGui::Indent(10.f);
					Refl::Class* new_class = render_node_types(child, filter, changed);

					if (!current && new_class)
						current = new_class;

					ImGui::Unindent(10.f);
					ImGui::TreePop();
				}
			}

			ImGui::PopID();
		}

		for (Refl::Struct* instance : group->structs())
		{
			if (instance->is_class())
			{
				if (!filter.empty())
				{
					String name = Strings::to_lower(instance->name().to_string());
					if (!name.starts_with(filter))
						continue;
				}

				if (ImGui::MenuItem(instance->name().c_str()))
				{
					current = Refl::Object::instance_cast<Refl::Class>(instance);
				}
			}
		}

		return current;
	}

	// 	bool MaterialEditorClient::show_new_node_popup(class VisualMaterial* material)
	// 	{
	// 		// const auto from = reinterpret_cast<VisualMaterialGraph::Pin*>(m_graph_state.m_create_node_from_pin);
	// 		// auto& state     = m_graph_state;


	// 		// ed::Resume();
	// 		return true;
	// 	}


	// 	// static void process_drag_and_drop(VisualMaterial* material)
	// 	// {
	// 	// 	if (ImGui::BeginDragDropTarget())
	// 	// 	{
	// 	// 		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
	// 	// 		if (payload)
	// 	// 		{
	// 	// 			IM_ASSERT(payload->DataSize == sizeof(Object*));
	// 	// 			Object* new_object = *reinterpret_cast<Object**>(payload->Data);
	// 	// 		}
	// 	// 		ImGui::EndDragDropTarget();
	// 	// 	}
	// 	// }

	// 	// static void process_editor_delete(VisualMaterial* material)
	// 	// {

	// 	// }

	inline VisualMaterialEditorClient& VisualMaterialEditorClient::on_node_select(VisualMaterialGraph::Node* node)
	{
		return *this;
	}

	inline VisualMaterialEditorClient& VisualMaterialEditorClient::on_node_create(VisualMaterialGraph::Node* node)
	{
		if (auto input = m_create_node_ctx.pin->as_input())
		{
			for (auto* output : node->outputs())
			{
				if (output->type() == input->type())
				{
					input->link(output);
					return *this;
				}
			}

			for (auto* output : node->outputs())
			{
				if (VisualMaterialGraph::Expression::is_compatible_types(output->type(), input->type()))
				{
					input->link(output);
					return *this;
				}
			}
		}
		else if (auto output = m_create_node_ctx.pin->as_output())
		{
			for (auto* input : node->inputs())
			{
				if (output->type() == input->type())
				{
					input->link(output);
					return *this;
				}
			}

			for (auto* input : node->inputs())
			{
				if (VisualMaterialGraph::Expression::is_compatible_types(output->type(), input->type()))
				{
					input->link(output);
					return *this;
				}
			}
		}
		return *this;
	}

	inline VisualMaterialEditorClient& VisualMaterialEditorClient::on_node_destroy(VisualMaterialGraph::Node* node)
	{
		return *this;
	}

	VisualMaterialEditorClient& VisualMaterialEditorClient::open_spawn_node_window(VisualMaterialGraph::Pin* pin)
	{
		if (!m_create_node_ctx.is_active)
		{
			ed::Suspend();
			ImGui::OpenPopup("###SpawnNode");

			m_create_node_ctx.pos       = ImGui::EngineVecFrom(ed::ScreenToCanvas(ImGui::GetMousePos()));
			m_create_node_ctx.filter    = "";
			m_create_node_ctx.pin       = pin;
			m_create_node_ctx.is_active = true;

			ed::Resume();
		}
		return *this;
	}

	VisualMaterialEditorClient& VisualMaterialEditorClient::render_default_pin_value(VisualMaterialGraph::Pin* pin)
	{
		return *this;
	}

	VisualMaterialEditorClient& VisualMaterialEditorClient::render_graph()
	{
		static BlueprintBuilder builder;

		float text_height                        = ImGui::GetTextLineHeightWithSpacing();
		float item_spacing                       = ImGui::GetStyle().ItemSpacing.x;
		VisualMaterialGraph::Node* selected_node = nullptr;

		auto selected_items = ed::GetSelectedObjectCount();

		for (auto& node : m_material->nodes())
		{
			auto pos = ed::GetNodePosition(node->id());

			if (pos.x == FLT_MAX || pos.y == FLT_MAX)
			{
				pos = {node->position.x, node->position.y};
				ed::SetNodePosition(node->id(), pos);
			}
			else
			{
				node->position = ImGui::EngineVecFrom(pos);
			}

			builder.begin(node->id());

			if (selected_items == 1 && ed::IsNodeSelected(node->id()))
			{
				selected_node = node;
			}

			builder.begin_header(ImVec4(1.0, 0.0, 0.0, 1.f));
			ImGui::Spring(1.f);
			ImGui::TextUnformatted(node->class_instance()->display_name().c_str());
			ImGui::Dummy({0, ImGui::GetTextLineHeightWithSpacing()});
			ImGui::Spring(1.f);
			builder.end_header();

			// Inputs rendering
			for (auto* input : node->inputs())
			{
				builder.begin_input(input->id());

				builder.begin_input_pin(input->id());
				ed::PinPivotAlignment({0.5, 0.5f});

				BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, input->links_count() > 0,
									   ImVec4(1.0, 0.0, 0.0, 1.0));
				builder.end_input_pin();

				ImGui::Text("%s", input->name().c_str());

				if (input->linked_pin() == nullptr)
				{
					if (auto default_value = input->default_value())
					{
						VerticalLayout layout;
						ImGui::PushItemWidth(180.f);
						s_default_type_renderers[default_value->type().type_index()](input);
						ImGui::PopItemWidth();
					}
				}
				builder.end_input();
			}

			// Content rendering
			builder.begin_middle();
			{
			}

			// Outputs rendering
			for (auto* output : node->outputs())
			{
				builder.begin_output(output->id());

				if (auto default_value = output->default_value())
				{
					VerticalLayout layout;
					ImGui::PushItemWidth(180.f);
					s_default_type_renderers[default_value->type().type_index()](output);
					ImGui::PopItemWidth();
				}

				ImGui::Text("%s", output->name().c_str());

				builder.begin_output_pin(output->id());
				ed::PinPivotAlignment({0.5, 0.5f});

				BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, output->links_count() > 0,
									   ImVec4(0.0, 1.0, 0.0, 1.0));
				builder.end_input_pin();
				builder.end_input();
			}

			// {
			// 	float max_len = 0.f;
			// 	static Vector<float> sizes;
			// 	sizes.clear();

			// 	for (auto& input : node->inputs())
			// 	{
			// 		float len = ImGui::CalcTextSize(input->name().c_str()).x;
			// 		max_len   = glm::max(max_len, len);
			// 		sizes.push_back(len);
			// 	}

			// 	Index index = 0;
			// 	for (auto& input : node->inputs())
			// 	{
			// 		builder.begin_input(input->id());

			// 		ImGui::Spring(0.f, 0.f);
			// 		builder.begin_input_pin(input->id());
			// 		ed::PinPivotAlignment({0.5, 0.5f});
			// 		const VisualMaterialGraph::PinType type = input->node()->in_pin_type(input);
			// 		BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, input->has_links(),
			// 		                       pin_colors.at(type));
			// 		ImGui::SetItemTooltip("%s", pin_type_names.at(type));
			// 		builder.end_input_pin();

			// 		ImGui::Spring(0.f, 0.f);
			// 		ImGui::TextUnformatted(input->name().c_str());

			// 		ImGui::Spring(0.f, 0.f);
			// 		ImGui::Dummy({item_spacing + max_len - sizes[index], 0.f});
			// 		ImGui::Spring(0.f, 0.f);

			// 		ImGui::SuspendLayout();
			// 		float width = render_default_value(input->default_value(), input->type());
			// 		ImGui::ResumeLayout();

			// 		ImGui::Spring(0.f, 0.f);
			// 		ImGui::Dummy({width, 0.f});
			// 		ImGui::Spring(1.f, 0.f);

			// 		builder.end_input();

			// 		++index;
			// 	}
			// }

			// ImGui::Spring();
			// builder.begin_middle();
			// node->render();
			// ImGui::Spring();


			// for (auto& output : node->outputs())
			// {
			// 	builder.begin_output(output->id());

			// 	ImGui::Spring(0.f, 0.f);
			// 	ImGui::SuspendLayout();
			// 	float width = render_default_value(output->default_value(), output->type());
			// 	ImGui::ResumeLayout();

			// 	ImGui::Spring(0.f, 0.f);
			// 	ImGui::Dummy({width, 0.f});
			// 	ImGui::Spring(1.f, 0.f);

			// 	ImGui::TextUnformatted(output->name().c_str());
			// 	ImGui::Spring(0.f, 0.f);

			// 	builder.begin_output_pin(output->id());
			// 	ed::PinPivotAlignment({0.9, 0.5f});
			// 	const VisualMaterialGraph::PinType type = output->node()->out_pin_type(output);
			// 	BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, output->has_links(),
			// 	                       pin_colors.at(type));
			// 	ImGui::SetItemTooltip("%s", pin_type_names.at(type));
			// 	builder.end_output_pin();

			// 	ImGui::Spring(0.f, 0.f);
			// 	builder.end_output();
			// }

			// if (node->has_error())
			// {
			// 	ImGui::Spring();
			// 	builder.begin_footer(ImVec4(1.f, 0.f, 0.f, 1.f));
			// 	ImGui::Spring(1.f);
			// 	ImGui::Text("%s", node->error_message().c_str());
			// 	ImGui::Spring(1.f);
			// }

			builder.end();
		}

		for (VisualMaterialGraph::Node* node : m_material->nodes())
		{
			for (auto* input : node->inputs())
			{
				if (auto* output = input->as_input()->linked_pin())
				{
					ed::Link(input->id() + 1, input->id(), output->id(), ImColor(0, 149, 220), 3.f);
				}
			}
		}

		return *this;
	}

	VisualMaterialEditorClient& VisualMaterialEditorClient::render_spawn_node_window()
	{
		if (!m_create_node_ctx.is_active)
			return *this;

		ed::Suspend();

		ImGui::SetNextWindowSizeConstraints({}, {300, 500});
		if ((m_create_node_ctx.is_active = ImGui::BeginPopup("###SpawnNode")))
		{
			ImGui::Dummy({200, 0});

			bool filter_changed = ImGui::InputTextWithHint("##SearchLine", "Search...", m_create_node_ctx.filter);
			filter_changed      = filter_changed || ImGui::IsWindowAppearing();

			ImGui::Separator();

			static Group* root_group = Group::find("Engine::VisualMaterialGraph::Nodes");
			if (Refl::Class* self = render_node_types(root_group, m_create_node_ctx.filter, filter_changed))
			{
				auto node = m_material->create_node(self, m_create_node_ctx.pos);

				if (m_create_node_ctx.pin)
				{
					on_node_create(node);
				}
			}
			ImGui::EndPopup();
		}

		ed::Resume();
		return *this;
	}

	VisualMaterialEditorClient& VisualMaterialEditorClient::update_create_events()
	{
		if (ed::BeginCreate(ImColor(0, 169, 233), 3.f))
		{
			ed::PinId from, to;
			if (ed::QueryNewLink(&from, &to) && from && to)
			{
				VisualMaterialGraph::Pin* input_pin  = from.AsPointer<VisualMaterialGraph::Pin>();
				VisualMaterialGraph::Pin* output_pin = to.AsPointer<VisualMaterialGraph::Pin>();

				if (input_pin->kind() != VisualMaterialGraph::Pin::Input)
				{
					std::swap(input_pin, output_pin);
				}

				if (input_pin == output_pin)
				{
					ed::RejectNewItem();
				}
				else if (input_pin->kind() == output_pin->kind())
				{
					show_label("editor/Cannot create link to same pin type"_localized, ImColor(255, 0, 0));
					ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), 3.f);
				}
				else if (input_pin->node() == output_pin->node())
				{
					show_label("editor/Cannot create link between pins of the same node"_localized, ImColor(255, 0, 0));
					ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), 3.f);
				}
				else if (!VisualMaterialGraph::Expression::is_compatible_types(output_pin->type(), input_pin->type()))
				{
					show_label("editor/Incompatible Pin Type"_localized, ImColor(255, 0, 0));
					ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), 3.f);
				}
				else if (ed::AcceptNewItem())
				{
					VisualMaterialGraph::InputPin* in   = static_cast<VisualMaterialGraph::InputPin*>(input_pin);
					VisualMaterialGraph::OutputPin* out = static_cast<VisualMaterialGraph::OutputPin*>(output_pin);
					in->link(out);
				}
			}

			if (ed::QueryNewNode(&from))
			{
				VisualMaterialGraph::Pin* pin = from.AsPointer<VisualMaterialGraph::Pin>();

				show_label("editor/+ Create Node"_localized, ImColor(32, 45, 32, 180));

				ed::NodeId node;
				if (ed::ShowNodeContextMenu(&node))
				{
					printf("CONTEX MENU!\n");
				}

				if (ed::AcceptNewItem() && !ImGui::IsPopupOpen("Create New Node"))
				{
					open_spawn_node_window(pin);
				}
			}
		}
		ed::EndCreate();
		return *this;
	}

	VisualMaterialEditorClient& VisualMaterialEditorClient::update_delete_events()
	{
		if (ed::BeginDelete())
		{
			ed::NodeId node_id = 0;

			while (ed::QueryDeletedNode(&node_id))
			{
				VisualMaterialGraph::Node* node = node_id.AsPointer<VisualMaterialGraph::Node>();

				if (node->class_instance()->is_a<VisualMaterialGraph::MaterialRoot>())
				{
					ed::RejectDeletedItem();
					continue;
				}

				if (ed::AcceptDeletedItem())
				{
					m_material->destroy_node(node);
				}
			}

			ed::LinkId link_id = 0;
			while (ed::QueryDeletedLink(&link_id))
			{
				VisualMaterialGraph::Pin* pin = link_id.AsPointer<VisualMaterialGraph::Pin>();
				if (pin)
				{
					ed::AcceptDeletedItem();
					pin->unlink();
				}
				else
				{
					ed::RejectDeletedItem();
				}
			}
		}
		ed::EndDelete();
		return *this;
	}

	// 	// static NodesSet copy_nodes(const NodesSet& nodes, const Vector2f& position_offset = {0, 0})
	// 	// {
	// 	// 	Map<VisualMaterialGraph::Node*, VisualMaterialGraph::Node*> m_nodes_map;
	// 	// 	NodesSet result;


	// 	// 	for (auto node : nodes)
	// 	// 	{
	// 	// 		if (!node->is_root_node())
	// 	// 		{
	// 	// 			auto copy         = Object::instance_cast<VisualMaterialGraph::Node>(Object::copy_from(node));
	// 	// 			copy->position    = node->position + position_offset;
	// 	// 			m_nodes_map[node] = copy;
	// 	// 			result.insert(copy);
	// 	// 		}
	// 	// 	}

	// 	// 	// Copy links
	// 	// 	for (auto& [src, dst] : m_nodes_map)
	// 	// 	{
	// 	// 		for (auto& input : src->inputs())
	// 	// 		{
	// 	// 			if (auto linked_to = input->linked_to())
	// 	// 			{
	// 	// 				if (auto node = m_nodes_map[linked_to->node()])
	// 	// 				{
	// 	// 					size_t in_index  = src->find_pin_index(input);
	// 	// 					size_t out_index = linked_to->node()->find_pin_index(linked_to);

	// 	// 					dst->input_pin(in_index)->create_link(node->output_pin(out_index));
	// 	// 				}
	// 	// 			}
	// 	// 		}
	// 	// 	}

	// 	// 	return result;
	// 	// }

	// 	// static NodesSet copy_selected_nodes(const Vector2f& position_offset = {0, 0})
	// 	// {
	// 	// 	NodesSet m_nodes;

	// 	// 	const int_t context_size = ed::GetSelectedObjectCount();
	// 	// 	Vector<ed::NodeId> nodes(context_size);
	// 	// 	int_t nodes_count = ed::GetSelectedNodes(nodes.data(), context_size);

	// 	// 	for (int_t i = 0; i < nodes_count; ++i)
	// 	// 	{
	// 	// 		m_nodes.insert(nodes[i].AsPointer<VisualMaterialGraph::Node>());
	// 	// 	}

	// 	// 	return copy_nodes(m_nodes, position_offset);
	// 	// }

	// 	// static NodesSet cut_selected_nodes(VisualMaterial* material)
	// 	// {
	// 	// 	NodesSet m_nodes;

	// 	// 	const int_t context_size = ed::GetSelectedObjectCount();
	// 	// 	Vector<ed::NodeId> nodes(context_size);

	// 	// 	int_t nodes_count = ed::GetSelectedNodes(nodes.data(), context_size);
	// 	// 	m_nodes.reserve(nodes_count);

	// 	// 	for (int_t i = 0; i < nodes_count; ++i)
	// 	// 	{
	// 	// 		auto node = nodes[i].AsPointer<VisualMaterialGraph::Node>();

	// 	// 		if (!node->is_root_node())
	// 	// 		{
	// 	// 			m_nodes.insert(node);
	// 	// 			material->destroy_node(node, false);
	// 	// 			ed::DeleteNode(node->id());
	// 	// 		}
	// 	// 	}

	// 	// 	for (auto& node : m_nodes)
	// 	// 	{
	// 	// 		for (auto& input : node->inputs())
	// 	// 		{
	// 	// 			if (auto linked_to = input->linked_to())
	// 	// 			{
	// 	// 				if (!m_nodes.contains(linked_to->node()))
	// 	// 				{
	// 	// 					input->unlink();
	// 	// 				}
	// 	// 			}
	// 	// 		}

	// 	// 		for (auto& output : node->outputs())
	// 	// 		{
	// 	// 			auto& linked_to = output->linked_to();

	// 	// 			for (auto input : linked_to)
	// 	// 			{
	// 	// 				if (!m_nodes.contains(input->node()))
	// 	// 				{
	// 	// 					input->unlink();
	// 	// 				}
	// 	// 			}
	// 	// 		}
	// 	// 	}

	// 	// 	return m_nodes;
	// 	// }

	VisualMaterialEditorClient& VisualMaterialEditorClient::update_events()
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Tab, false))
			open_spawn_node_window();

		update_create_events().update_delete_events();

		auto& io = ImGui::GetIO();

		// Select All
		if (io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_A))
		{
			ed::ClearSelection();
			for (auto node : m_material->nodes())
			{
				ed::SelectNode(node->id(), true);
			}
		}

		// if (io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_X))
		// {
		// 	m_graph_state.m_nodes = cut_selected_nodes(material);
		// }

		// // Copy command
		// if (io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_C))
		// {
		// 	m_graph_state.m_nodes = copy_selected_nodes({0, 0});
		// }

		// // Paste command
		// if (io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_V))
		// {
		// 	if (!m_graph_state.m_nodes.empty())
		// 	{
		// 		ed::ClearSelection();
		// 		auto nodes = copy_nodes(m_graph_state.m_nodes);

		// 		Vector2f mid_point = {0, 0};

		// 		for (auto& node : nodes)
		// 		{
		// 			material->register_node(node);
		// 			mid_point += node->position;
		// 		}

		// 		mid_point /= static_cast<float>(m_graph_state.m_nodes.size());

		// 		Vector2f difference = ImGui::EngineVecFrom(ImGui::GetMousePos()) - mid_point;

		// 		for (auto& node : nodes)
		// 		{
		// 			node->position += difference;
		// 			ed::SelectNode(node->id(), true, true);
		// 			ed::SetNodePosition(node->id(), {node->position.x, node->position.y});
		// 		}
		// 	}
		// }

		// // Duplicate command
		// if (io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_D))
		// {
		// 	auto new_nodes = copy_selected_nodes({20, 20});

		// 	if (!new_nodes.empty())
		// 	{
		// 		ed::ClearSelection();

		// 		for (VisualMaterialGraph::Node* copy : new_nodes)
		// 		{
		// 			material->register_node(copy);
		// 			ed::SelectNode(copy->id(), true, true);
		// 			ed::SetNodePosition(copy->id(), {copy->position.x, copy->position.y});
		// 		}
		// 	}
		// }

		return *this;
	}

	VisualMaterialEditorClient::VisualMaterialEditorClient()
	{
		ax::NodeEditor::Config config;
		config.SettingsFile = nullptr;
		m_context           = ax::NodeEditor::CreateEditor(&config);

		menu_bar.create("")->actions.push([this]() {
			if (m_material && ImGui::MenuItem("Dump Source"))
			{
				String source;
				m_material->shader_source(source);
				printf("%s\n", source.c_str());
			}
		});
	}

	VisualMaterialEditorClient::~VisualMaterialEditorClient()
	{
		ax::NodeEditor::DestroyEditor(m_context);
	}

	VisualMaterialEditorClient& VisualMaterialEditorClient::update(float dt)
	{
		Super::update(dt);

		ImGui::Begin("editor/Graph###Graph"_localized);

		if (ImGui::IsWindowAppearing())
		{
			ImGui::End();
			return *this;
		}

		ax::NodeEditor::SetCurrentEditor(m_context);
		ax::NodeEditor::Begin("Editor");

		if (m_material)
		{
			render_graph().update_events().render_spawn_node_window();
		}

		ax::NodeEditor::End();

		if (m_material)
		{
			//process_drag_and_drop(material);
		}

		ImGui::End();
		return *this;
	}

	VisualMaterialEditorClient& VisualMaterialEditorClient::select(Object* object)
	{
		if (VisualMaterial* material = instance_cast<VisualMaterial>(object))
		{
			m_material = material;
			Super::select(material);
		}
		return *this;
	}

	uint32_t VisualMaterialEditorClient::build_dock(uint32_t dock)
	{
		auto center = Super::build_dock(dock);
		ImGui::DockBuilderDockWindow("###Graph", center);
		return center;
	}

	static void pre_initialize()
	{
		using T = ShaderParameterType;

		s_default_type_renderers[T(T::Bool).type_index()]  = render_default_vector1b;
		s_default_type_renderers[T(T::Bool2).type_index()] = render_default_vector2b;
		s_default_type_renderers[T(T::Bool3).type_index()] = render_default_vector3b;
		s_default_type_renderers[T(T::Bool4).type_index()] = render_default_vector4b;

		s_default_type_renderers[T(T::Int).type_index()]  = render_default_vector1i;
		s_default_type_renderers[T(T::Int2).type_index()] = render_default_vector2i;
		s_default_type_renderers[T(T::Int3).type_index()] = render_default_vector3i;
		s_default_type_renderers[T(T::Int4).type_index()] = render_default_vector4i;

		s_default_type_renderers[T(T::UInt).type_index()]  = render_default_vector1u;
		s_default_type_renderers[T(T::UInt2).type_index()] = render_default_vector2u;
		s_default_type_renderers[T(T::UInt3).type_index()] = render_default_vector3u;
		s_default_type_renderers[T(T::UInt4).type_index()] = render_default_vector4u;

		s_default_type_renderers[T(T::Float).type_index()]  = render_default_vector1f;
		s_default_type_renderers[T(T::Float2).type_index()] = render_default_vector2f;
		s_default_type_renderers[T(T::Float3).type_index()] = render_default_vector3f;
		s_default_type_renderers[T(T::Float4).type_index()] = render_default_vector4f;
	}

	static PreInitializeController preinit(pre_initialize);
}// namespace Engine
