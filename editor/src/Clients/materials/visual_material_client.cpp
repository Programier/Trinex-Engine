#include <Clients/material/visual_material_client.hpp>
#include <Core/blueprints.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/group.hpp>
#include <Core/icons.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_nodes.hpp>
#include <RHI/rhi.hpp>
#include <Widgets/property_renderer.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	trinex_implement_class(Engine::VisualMaterialEditorClient, 0)
	{
		register_client(VisualMaterial::static_reflection(), static_reflection());
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

	class NodePropertyLayout
	{

	public:
		NodePropertyLayout(float weight = 0.f)
		{
			ImGui::BeginHorizontal(1);
			ImGui::Spring(weight, 0.5f);
		}

		~NodePropertyLayout()
		{
			ImGui::Spring(1.f, 0.5f);
			ImGui::EndHorizontal();
		}
	};

	struct EditorSuspend {
		EditorSuspend() { ed::Suspend(); }
		~EditorSuspend() { ed::Resume(); }
	};

	template<int N>
	static void render_vector_nb(void* data)
	{
		static_assert(N > 0);
		auto* window = ImGui::GetCurrentWindow();

		if (window->SkipItems)
			return;

		if constexpr (N == 1)
		{
			ImGui::Checkbox("##vector1b", reinterpret_cast<bool*>(data));
		}
		else
		{
			auto& style = ImGui::GetStyle();

			ImGui::BeginGroup();
			ImGui::PushMultiItemsWidths(N, ImGui::CalcItemWidth());

			for (int i = 0; i < N; i++)
			{
				ImGui::PushID(i);
				if (i > 0)
					ImGui::SameLine(0, style.ItemInnerSpacing.x);

				ImGui::Checkbox("##vectornb", reinterpret_cast<bool*>(data) + i);
				ImGui::PopID();
				ImGui::PopItemWidth();
			}

			ImGui::PopID();
			ImGui::EndGroup();
		}
	}

	template<int N, ImGuiDataType T, typename Element>
	static void render_vector_nt(void* data)
	{
		static constexpr const char* data_type_formats[] = {
		        "%hhd", // ImGuiDataType_S8
		        "%hhu", // ImGuiDataType_U8
		        "%hd",  // ImGuiDataType_S16
		        "%hu",  // ImGuiDataType_U16
		        "%d",   // ImGuiDataType_S32
		        "%u",   // ImGuiDataType_U32
		        "%lld", // ImGuiDataType_S64
		        "%llu", // ImGuiDataType_U64
		        "%.3f", // ImGuiDataType_Float
		        "%.3lf",// ImGuiDataType_Double
		};

		constexpr const char* element_format = data_type_formats[T];
		float max_length                     = 0;

		for (int i = 0; i < N; ++i)
		{
			char buffer[64];
			snprintf(buffer, sizeof(buffer), element_format, reinterpret_cast<Element*>(data)[i]);
			max_length = glm::max(max_length, ImGui::CalcTextSize(buffer).x);
		}

		float padding     = ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = (max_length + padding) * N + ImGui::GetStyle().ItemInnerSpacing.x * (N - 1);

		ImGui::SetNextItemWidth(total_width);
		ImGui::InputScalarN("###vector_nt", T, data, N, nullptr, nullptr, element_format);
	}

	static void (*s_default_type_renderers[17])(void* value) = {nullptr};

	template<typename T>
	static int_t find_max_pin_name_len(const T& pins)
	{
		int_t len = 0;
		for (VisualMaterialGraph::Pin* pin : pins)
		{
			len = glm::max<int_t>(pin->name().length(), len);
		}
		return len;
	}

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

	VisualMaterialEditorClient& VisualMaterialEditorClient::render_graph()
	{
		static BlueprintBuilder builder;
		float text_height = ImGui::GetTextLineHeightWithSpacing();

		m_selected_nodes.clear();

		for (auto& node : m_material->nodes())
		{
			const ed::NodeId node_id = ed::NodeId(node.ptr());

			auto pos = ed::GetNodePosition(node_id);

			if (pos.x == FLT_MAX || pos.y == FLT_MAX)
			{
				pos = {node->position.x, node->position.y};
				ed::SetNodePosition(node_id, pos);
			}
			else
			{
				node->position = ImGui::EngineVecFrom(pos);
			}

			builder.begin(node_id);

			if (ed::IsNodeSelected(node_id))
				m_selected_nodes.push_back(node);

			builder.begin_header(ImVec4(1.0, 0.0, 0.0, 1.f));
			ImGui::Spring(1.f);
			ImGui::TextUnformatted(node->class_instance()->display_name().c_str());
			ImGui::Dummy({0, ImGui::GetTextLineHeightWithSpacing()});
			ImGui::Spring(1.f);
			builder.end_header();

			const int_t max_input_name_len  = find_max_pin_name_len(node->inputs());
			const int_t max_output_name_len = find_max_pin_name_len(node->outputs());

			// Inputs rendering
			for (auto* input : node->inputs())
			{
				builder.begin_input();

				builder.begin_input_pin(ed::PinId(input));
				ed::PinPivotAlignment({0.5, 0.5f});

				BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, input->links_count() > 0,
				                       ImVec4(1.0, 0.0, 0.0, 1.0));
				builder.end_input_pin();

				ImGui::Text("%*s ", -max_input_name_len, input->name().c_str());

				if (input->linked_pin() == nullptr)
				{
					if (auto default_value = input->default_value())
					{
						VerticalLayout layout;
						s_default_type_renderers[default_value->type().type_index()](input->default_value()->address());
					}
				}

				builder.end_input();
			}

			ImGui::Dummy({ImGui::GetTextLineHeightWithSpacing(), 0.f});

			// Content rendering
			builder.begin_middle();
			{
				node->render();
			}

			// Outputs rendering
			for (auto* output : node->outputs())
			{
				builder.begin_output();

				if (auto default_value = output->default_value())
				{
					VerticalLayout layout;
					s_default_type_renderers[default_value->type().type_index()](output->default_value()->address());
				}

				ImGui::Text(" %*s", max_output_name_len, output->name().c_str());

				builder.begin_output_pin(ed::PinId(output));
				ed::PinPivotAlignment({0.5, 0.5f});

				BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, output->links_count() > 0,
				                       ImVec4(0.0, 1.0, 0.0, 1.0));
				builder.end_input_pin();
				builder.end_input();
			}

			builder.end();
		}

		for (VisualMaterialGraph::Node* node : m_material->nodes())
		{
			for (auto* input : node->inputs())
			{
				if (auto* output = input->as_input()->linked_pin())
				{
					ed::Link(ed::LinkId(input), ed::PinId(input), ed::PinId(output), ImColor(0, 149, 220), 3.f);
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
				ed::SelectNode(ed::NodeId(node.ptr()), true);
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

		if (m_properties_window)
		{
			Object* properties_object = nullptr;

			if (m_properties_window && m_selected_nodes.size() == 1)
			{
				auto node = m_selected_nodes.front();

				if (m_properties_window->properties_map(node->class_instance()).empty())
				{
					properties_object = m_material;
				}
				else
				{
					properties_object = node;
				}
			}
			else
			{
				properties_object = m_material;
			}

			m_properties_window->object(properties_object, false);
		}
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

	VisualMaterialEditorClient& VisualMaterialEditorClient::create_properties_window()
	{
		Super::create_properties_window();
		return *this;
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

	//////////////////// INITITALIZATION ////////////////////

	static void pre_initialize()
	{
		using T = RHIShaderParameterType;

		s_default_type_renderers[T(T::Bool).type_index()]  = render_vector_nb<1>;
		s_default_type_renderers[T(T::Bool2).type_index()] = render_vector_nb<2>;
		s_default_type_renderers[T(T::Bool3).type_index()] = render_vector_nb<3>;
		s_default_type_renderers[T(T::Bool4).type_index()] = render_vector_nb<4>;

		s_default_type_renderers[T(T::Int).type_index()]  = render_vector_nt<1, ImGuiDataType_S32, int32_t>;
		s_default_type_renderers[T(T::Int2).type_index()] = render_vector_nt<2, ImGuiDataType_S32, int32_t>;
		s_default_type_renderers[T(T::Int3).type_index()] = render_vector_nt<3, ImGuiDataType_S32, int32_t>;
		s_default_type_renderers[T(T::Int4).type_index()] = render_vector_nt<4, ImGuiDataType_S32, int32_t>;

		s_default_type_renderers[T(T::UInt).type_index()]  = render_vector_nt<1, ImGuiDataType_U32, uint32_t>;
		s_default_type_renderers[T(T::UInt2).type_index()] = render_vector_nt<2, ImGuiDataType_U32, uint32_t>;
		s_default_type_renderers[T(T::UInt3).type_index()] = render_vector_nt<3, ImGuiDataType_U32, uint32_t>;
		s_default_type_renderers[T(T::UInt4).type_index()] = render_vector_nt<4, ImGuiDataType_U32, uint32_t>;

		s_default_type_renderers[T(T::Float).type_index()]  = render_vector_nt<1, ImGuiDataType_Float, float>;
		s_default_type_renderers[T(T::Float2).type_index()] = render_vector_nt<2, ImGuiDataType_Float, float>;
		s_default_type_renderers[T(T::Float3).type_index()] = render_vector_nt<3, ImGuiDataType_Float, float>;
		s_default_type_renderers[T(T::Float4).type_index()] = render_vector_nt<4, ImGuiDataType_Float, float>;
	}

	static PreInitializeController preinit(pre_initialize);
}// namespace Engine
