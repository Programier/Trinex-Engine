#include <Clients/material_editor_client.hpp>
#include <Core/blueprints.hpp>
#include <Core/class.hpp>
#include <Core/group.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>
#include <imgui_stacklayout.h>

namespace Engine
{
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

	static const TreeMap<VisualMaterialGraph::PinType, ImVec4> pin_colors = {
	        {VisualMaterialGraph::PinType::Undefined, ImVec4(0.0, 0.0, 1.0, 1.0)},
	        {VisualMaterialGraph::PinType::Bool, ImVec4(0.0, 0.0, 1.0, 1.0)},
	        {VisualMaterialGraph::PinType::Int, ImVec4(1.0, 0.0, 0.0, 1.0)},
	        {VisualMaterialGraph::PinType::UInt, ImVec4(1.0, 0.647, 0.0, 1.0)},
	        {VisualMaterialGraph::PinType::Float, ImVec4(0.0, 1.0, 0.0, 1.0)},
	        {VisualMaterialGraph::PinType::BVec2, ImVec4(1.0, 0.753, 0.796, 1.0)},
	        {VisualMaterialGraph::PinType::BVec3, ImVec4(1.0, 0.078, 0.576, 1.0)},
	        {VisualMaterialGraph::PinType::BVec4, ImVec4(1.0, 0.412, 0.706, 1.0)},
	        {VisualMaterialGraph::PinType::IVec2, ImVec4(1.0, 1.0, 0.0, 1.0)},
	        {VisualMaterialGraph::PinType::IVec3, ImVec4(0.855, 0.647, 0.125, 1.0)},
	        {VisualMaterialGraph::PinType::IVec4, ImVec4(1.0, 0.843, 0.0, 1.0)},
	        {VisualMaterialGraph::PinType::UVec2, ImVec4(0.0, 1.0, 1.0, 1.0)},
	        {VisualMaterialGraph::PinType::UVec3, ImVec4(0.255, 0.412, 0.882, 1.0)},
	        {VisualMaterialGraph::PinType::UVec4, ImVec4(0.0, 0.749, 1.0, 1.0)},
	        {VisualMaterialGraph::PinType::Vec2, ImVec4(1.0, 0.0, 1.0, 1.0)},
	        {VisualMaterialGraph::PinType::Vec3, ImVec4(0.502, 0.0, 0.502, 1.0)},
	        {VisualMaterialGraph::PinType::Color3, ImVec4(1.0, 0.078, 0.576, 1.0)},
	        {VisualMaterialGraph::PinType::Vec4, ImVec4(0.502, 0.0, 0.0, 1.0)},
	        {VisualMaterialGraph::PinType::Color4, ImVec4(1.0, 0.412, 0.706, 1.0)},
	        {VisualMaterialGraph::PinType::Mat3, ImVec4(1.0, 0.843, 0.0, 1.0)},
	        {VisualMaterialGraph::PinType::Mat4, ImVec4(0.855, 0.647, 0.125, 1.0)},
	        {VisualMaterialGraph::PinType::Sampler, ImVec4(0.0, 0.502, 0.502, 1.0)},
	        {VisualMaterialGraph::PinType::Texture2D, ImVec4(0.0, 0.502, 0.0, 1.0)},
	};
	static const TreeMap<VisualMaterialGraph::PinType, const char*> pin_type_names = {
	        {VisualMaterialGraph::PinType::Undefined, "Undefined"},
	        {VisualMaterialGraph::PinType::Bool, "bool"},
	        {VisualMaterialGraph::PinType::Int, "int"},
	        {VisualMaterialGraph::PinType::UInt, "uint"},
	        {VisualMaterialGraph::PinType::Float, "float"},
	        {VisualMaterialGraph::PinType::BVec2, "bvec2"},
	        {VisualMaterialGraph::PinType::BVec3, "bvec3"},
	        {VisualMaterialGraph::PinType::BVec4, "bvec4"},
	        {VisualMaterialGraph::PinType::IVec2, "ivec2"},
	        {VisualMaterialGraph::PinType::IVec3, "ivec2"},
	        {VisualMaterialGraph::PinType::IVec4, "ivec4"},
	        {VisualMaterialGraph::PinType::UVec2, "uvec2"},
	        {VisualMaterialGraph::PinType::UVec3, "uvec3"},
	        {VisualMaterialGraph::PinType::UVec4, "uvec4"},
	        {VisualMaterialGraph::PinType::Vec2, "vec2"},
	        {VisualMaterialGraph::PinType::Vec3, "vec3"},
	        {VisualMaterialGraph::PinType::Color3, "color3"},
	        {VisualMaterialGraph::PinType::Vec4, "vec4"},
	        {VisualMaterialGraph::PinType::Color4, "color4"},
	        {VisualMaterialGraph::PinType::Mat3, "mat3"},
	        {VisualMaterialGraph::PinType::Mat4, "mat4"},
	        {VisualMaterialGraph::PinType::Sampler, "Sampler"},
	        {VisualMaterialGraph::PinType::Texture2D, "Texture2D"},
	};

	static FORCE_INLINE float input_item_width(int components)
	{
		return 75.f * static_cast<float>(components);
	}

	static void render_bool_pin(bool* data, uint32_t count)
	{
		static const char* names[] = {"##Value1", "##Value2", "##Value3", "##Value4"};
		for (uint32_t i = 0; i < count; ++i)
		{
			ImGui::Checkbox(names[i], data + i);

			if (i != count - 1)
				ImGui::SameLine();
		}
	}

	static float render_default_value(void* data, VisualMaterialGraph::PinType type)
	{
		if (data == nullptr)
			return 0;

		union Storage
		{
			void* ptr;
			bool* bool_ptr;
			float* float_ptr;

			Storage(void* data) : ptr(data)
			{}
		} storage(data);

		ImGui::BeginGroup();

		switch (type)
		{
			case VisualMaterialGraph::PinType::Bool:
				ImGui::Checkbox("##Value", storage.bool_ptr);
				break;

			case VisualMaterialGraph::PinType::Int:
				ImGui::SetNextItemWidth(input_item_width(1));
				ImGui::InputScalar("##Value", ImGuiDataType_S32, storage.ptr);
				break;

			case VisualMaterialGraph::PinType::UInt:
				ImGui::SetNextItemWidth(input_item_width(1));
				ImGui::InputScalar("##Value", ImGuiDataType_U32, storage.ptr);
				break;

			case VisualMaterialGraph::PinType::Float:
				ImGui::SetNextItemWidth(input_item_width(1));
				ImGui::InputScalar("##Value", ImGuiDataType_Float, storage.ptr);
				break;

			case VisualMaterialGraph::PinType::BVec2:
				ImGui::SetNextItemWidth(input_item_width(2));
				render_bool_pin(storage.bool_ptr, 2);
				break;

			case VisualMaterialGraph::PinType::BVec3:
				ImGui::SetNextItemWidth(input_item_width(3));
				render_bool_pin(storage.bool_ptr, 3);
				break;

			case VisualMaterialGraph::PinType::BVec4:
				ImGui::SetNextItemWidth(input_item_width(4));
				render_bool_pin(storage.bool_ptr, 4);
				break;
			case VisualMaterialGraph::PinType::IVec2:
				ImGui::SetNextItemWidth(input_item_width(2));
				ImGui::InputScalarN("##Value", ImGuiDataType_S32, storage.ptr, 2);
				break;
			case VisualMaterialGraph::PinType::IVec3:
				ImGui::SetNextItemWidth(input_item_width(3));
				ImGui::InputScalarN("##Value", ImGuiDataType_S32, storage.ptr, 3);
				break;
			case VisualMaterialGraph::PinType::IVec4:
				ImGui::SetNextItemWidth(input_item_width(4));
				ImGui::InputScalarN("##Value", ImGuiDataType_S32, storage.ptr, 4);
				break;
			case VisualMaterialGraph::PinType::UVec2:
				ImGui::SetNextItemWidth(input_item_width(2));
				ImGui::InputScalarN("##Value", ImGuiDataType_U32, storage.ptr, 2);
				break;
			case VisualMaterialGraph::PinType::UVec3:
				ImGui::SetNextItemWidth(input_item_width(3));
				ImGui::InputScalarN("##Value", ImGuiDataType_U32, storage.ptr, 3);
				break;
			case VisualMaterialGraph::PinType::UVec4:
				ImGui::SetNextItemWidth(input_item_width(4));
				ImGui::InputScalarN("##Value", ImGuiDataType_U32, storage.ptr, 4);
				break;
			case VisualMaterialGraph::PinType::Vec2:
				ImGui::SetNextItemWidth(input_item_width(2));
				ImGui::InputScalarN("##Value", ImGuiDataType_Float, storage.ptr, 2);
				break;
			case VisualMaterialGraph::PinType::Vec3:
				ImGui::SetNextItemWidth(input_item_width(3));
				ImGui::InputScalarN("##Value", ImGuiDataType_Float, storage.ptr, 3);
				break;
			case VisualMaterialGraph::PinType::Color3:
				ImGui::SetNextItemWidth(input_item_width(3));
				ImGui::ColorEdit3("##Value", storage.float_ptr);
				break;
			case VisualMaterialGraph::PinType::Vec4:
				ImGui::SetNextItemWidth(input_item_width(4));
				ImGui::InputScalarN("##Value", ImGuiDataType_Float, storage.ptr, 4);
				break;
			case VisualMaterialGraph::PinType::Color4:
				ImGui::SetNextItemWidth(input_item_width(4));
				ImGui::ColorEdit4("##Value", storage.float_ptr);
				break;
			case VisualMaterialGraph::PinType::Mat3:
				break;
			case VisualMaterialGraph::PinType::Mat4:
				break;
			default:
				break;
		}

		ImGui::EndGroup();
		return ImGui::GetItemRectSize().x;
	}


	static void render_graph(const Vector<Pointer<VisualMaterialGraph::Node>>& nodes)
	{
		static BlueprintBuilder builder;

		float text_height  = ImGui::GetTextLineHeightWithSpacing();
		float item_spacing = ImGui::GetStyle().ItemSpacing.x;

		for (auto& node : nodes)
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

			builder.begin_header(ImGui::ImVecFrom(node->header_color()));
			ImGui::Spring(1.f);
			ImGui::TextUnformatted(node->name());
			ImGui::Dummy({0, ImGui::GetTextLineHeightWithSpacing()});
			ImGui::Spring(1.f);
			builder.end_header();

			{
				float max_len = 0.f;
				static Vector<float> sizes;
				sizes.clear();

				for (auto& input : node->inputs())
				{
					float len = ImGui::CalcTextSize(input->name().c_str()).x;
					max_len   = glm::max(max_len, len);
					sizes.push_back(len);
				}

				Index index = 0;
				for (auto& input : node->inputs())
				{
					builder.begin_input(input->id());

					ImGui::Spring(0.f, 0.f);
					builder.begin_input_pin(input->id());
					ed::PinPivotAlignment({0.5, 0.5f});
					const VisualMaterialGraph::PinType type = input->node()->in_pin_type(input);
					BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, input->has_links(),
					                       pin_colors.at(type));
					ImGui::SetItemTooltip("%s", pin_type_names.at(type));
					builder.end_input_pin();

					ImGui::Spring(0.f, 0.f);
					ImGui::TextUnformatted(input->name().c_str());

					ImGui::Spring(0.f, 0.f);
					ImGui::Dummy({item_spacing + max_len - sizes[index], 0.f});
					ImGui::Spring(0.f, 0.f);

					ImGui::SuspendLayout();
					float width = render_default_value(input->default_value(), input->type());
					ImGui::ResumeLayout();

					ImGui::Spring(0.f, 0.f);
					ImGui::Dummy({width, 0.f});
					ImGui::Spring(1.f, 0.f);

					builder.end_input();

					++index;
				}
			}

			ImGui::Spring();
			builder.begin_middle();
			node->render();
			ImGui::Spring();


			for (auto& output : node->outputs())
			{
				builder.begin_output(output->id());

				ImGui::Spring(0.f, 0.f);
				ImGui::SuspendLayout();
				float width = render_default_value(output->default_value(), output->type());
				ImGui::ResumeLayout();

				ImGui::Spring(0.f, 0.f);
				ImGui::Dummy({width, 0.f});
				ImGui::Spring(1.f, 0.f);

				ImGui::TextUnformatted(output->name().c_str());
				ImGui::Spring(0.f, 0.f);

				builder.begin_output_pin(output->id());
				ed::PinPivotAlignment({0.9, 0.5f});
				const VisualMaterialGraph::PinType type = output->node()->out_pin_type(output);
				BlueprintBuilder::icon({text_height, text_height}, BlueprintBuilder::IconType::Circle, output->has_links(),
				                       pin_colors.at(type));
				ImGui::SetItemTooltip("%s", pin_type_names.at(type));
				builder.end_output_pin();

				ImGui::Spring(0.f, 0.f);
				builder.end_output();
			}

			if (node->has_error())
			{
				ImGui::Spring();
				builder.begin_footer(ImVec4(1.f, 0.f, 0.f, 1.f));
				ImGui::Spring(1.f);
				ImGui::Text("%s", node->error_message().c_str());
				ImGui::Spring(1.f);
			}

			builder.end();
		}

		for (VisualMaterialGraph::Node* node : nodes)
		{
			for (auto* input : node->inputs())
			{
				if (auto* output = input->linked_to())
				{
					ed::Link(input->id() + 1, input->id(), output->id(), ImColor(0, 149, 220), 2.f);
				}
			}
		}
	}

	static void open_nodes_popup(MaterialEditorClient::GraphState& state, bool is_in_canvas)
	{
		if (is_in_canvas)
			state.m_node_spawn_position = ImGui::EngineVecFrom(ImGui::GetMousePos());
		else
			state.m_node_spawn_position = ImGui::EngineVecFrom(ed::ScreenToCanvas(ImGui::GetMousePos()));
		ed::Suspend();
		ImGui::OpenPopup("Create New Node");
		ed::Resume();
	}

	static void check_creating(void*& out_pin, MaterialEditorClient::GraphState& state)
	{
		if (ed::BeginCreate(ImColor(0, 169, 233), 2.f))
		{
			ed::PinId from, to;
			if (ed::QueryNewLink(&from, &to) && from && to)
			{
				VisualMaterialGraph::Pin* input_pin  = from.AsPointer<VisualMaterialGraph::Pin>();
				VisualMaterialGraph::Pin* output_pin = to.AsPointer<VisualMaterialGraph::Pin>();

				if (input_pin->kind() != VisualMaterialGraph::PinKind::Input)
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
				else if (!input_pin->node()->can_connect(
				                 reinterpret_cast<VisualMaterialGraph::InputPin*>(input_pin),
				                 output_pin->node()->out_pin_type(reinterpret_cast<VisualMaterialGraph::OutputPin*>(output_pin))))
				{
					show_label("editor/Incompatible Pin Type"_localized, ImColor(255, 0, 0));
					ed::RejectNewItem(ImVec4(1.0f, 0.f, 0.f, 1.f), 3.f);
				}
				else if (ed::AcceptNewItem())
				{
					VisualMaterialGraph::InputPin* in   = reinterpret_cast<VisualMaterialGraph::InputPin*>(input_pin);
					VisualMaterialGraph::OutputPin* out = reinterpret_cast<VisualMaterialGraph::OutputPin*>(output_pin);
					in->create_link(out);
				}
			}

			if (ed::QueryNewNode(&from))
			{
				VisualMaterialGraph::Pin* pin = from.AsPointer<VisualMaterialGraph::Pin>();
				show_label("editor/+ Create Node"_localized, ImColor(32, 45, 32, 180));

				if (ed::AcceptNewItem() && !ImGui::IsPopupOpen("Create New Node"))
				{
					open_nodes_popup(state, false);
					out_pin = pin;
				}
			}
		}
		ed::EndCreate();
	}


	static Class* render_node_types(Group* group)
	{
		if (!group)
			return nullptr;


		Class* current = nullptr;

		for (Group* child : group->childs())
		{
			if (ImGui::CollapsingHeader(child->name().c_str()))
			{
				ImGui::Indent(10.f);
				Class* new_class = render_node_types(child);
				if (!current && new_class)
					current = new_class;

				ImGui::Unindent(10.f);
			}
		}


		for (Struct* instance : group->structs())
		{
			if (instance->is_class())
			{
				if (ImGui::MenuItem(instance->base_name().c_str()))
				{
					current = reinterpret_cast<Class*>(instance);
				}
			}
		}

		return current;
	}

	static bool show_new_node_popup(VisualMaterial* material, VisualMaterialGraph::Pin* from,
	                                MaterialEditorClient::GraphState& state)
	{
		bool status = false;
		ed::Suspend();
		ImGui::SetNextWindowSizeConstraints({}, {300, 500});
		if ((status = ImGui::BeginPopup("Create New Node")))
		{
			ImGui::Dummy({200, 0});

			static Group* root_group = Group::find("Engine::VisualMaterialGraphGroups");
			if (Class* self = render_node_types(root_group))
			{
				auto node      = material->create_node(self);
				node->position = state.m_node_spawn_position;

				if (from)
				{
					if (from->kind() == VisualMaterialGraph::PinKind::Input)
					{
						for (auto& output : node->outputs())
						{
							if (output->type() == from->type())
							{
								reinterpret_cast<VisualMaterialGraph::InputPin*>(from)->create_link(output);
								break;
							}
						}

						for (auto& output : node->outputs())
						{
							if (VisualMaterialGraph::is_convertable(output->type(), from->type()))
							{
								reinterpret_cast<VisualMaterialGraph::InputPin*>(from)->create_link(output);
								break;
							}
						}
					}
					else
					{
						for (auto& input : node->inputs())
						{
							if (input->type() == from->type())
							{
								input->create_link(reinterpret_cast<VisualMaterialGraph::OutputPin*>(from));
								break;
							}
						}

						for (auto& input : node->inputs())
						{
							if (VisualMaterialGraph::is_convertable(input->type(), from->type()))
							{
								input->create_link(reinterpret_cast<VisualMaterialGraph::OutputPin*>(from));
								break;
							}
						}
					}
				}
			}

			ImGui::EndPopup();
		}

		ed::Resume();
		return status;
	}


	static void delete_selected_items(VisualMaterial* material)
	{
		size_t objects = ed::GetSelectedObjectCount();
		if (objects == 0)
			return;

		byte* _data = new byte[objects * glm::max(sizeof(ed::NodeId), sizeof(ed::PinId))];

		{
			ed::NodeId* nodes = reinterpret_cast<ed::NodeId*>(_data);
			for (int i = 0, count = ed::GetSelectedNodes(nodes, objects); i < count; i++)
			{
				VisualMaterialGraph::Node* node = nodes[i].AsPointer<VisualMaterialGraph::Node>();

				if (node->is_destroyable())
				{
					ed::DeleteNode(nodes[i]);
					material->destroy_node(node);
				}
			}
		}

		{
			ed::LinkId* links = reinterpret_cast<ed::LinkId*>(_data);
			for (int i = 0, count = ed::GetSelectedLinks(links, objects); i < count; i++)
			{
				VisualMaterialGraph::Pin* pin =
				        reinterpret_cast<VisualMaterialGraph::Pin*>(static_cast<Identifier>(links[i]) - 1);

				if (pin)
				{
					pin->unlink();
				}
			}
		}

		delete[] _data;
		ed::ClearSelection();
	}


	static void process_drag_and_drop(VisualMaterial* material)
	{
		if (ImGui::BeginDragDropTarget())
		{

			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
			if (payload)
			{
				IM_ASSERT(payload->DataSize == sizeof(Object*));
				Object* new_object = *reinterpret_cast<Object**>(payload->Data);

				if (Texture2D* texture = Object::instance_cast<Texture2D>(new_object))
				{
					auto pos       = ed::ScreenToCanvas(ImGui::GetMousePos());
					auto node      = material->create_node<VisualMaterialGraph::Texture2D>();
					node->texture  = texture;
					node->position = Vector2D(pos.x, pos.y);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	MaterialEditorClient& MaterialEditorClient::render_visual_material_graph(class VisualMaterial* material)
	{
		ax::NodeEditor::SetCurrentEditor(m_graph_editor_context);
		ax::NodeEditor::Begin("Editor");

		if (material)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Tab, false))
			{
				open_nodes_popup(m_graph_state, true);
			}

			render_graph(material->nodes());
			check_creating(m_create_node_from_pin, m_graph_state);
			if (!show_new_node_popup(material, reinterpret_cast<VisualMaterialGraph::Pin*>(m_create_node_from_pin),
			                         m_graph_state))
			{
				m_create_node_from_pin = nullptr;
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
			{
				delete_selected_items(material);
			}
		}
		ax::NodeEditor::End();

		if (material)
		{
			process_drag_and_drop(material);
		}
		return *this;
	}
}// namespace Engine
