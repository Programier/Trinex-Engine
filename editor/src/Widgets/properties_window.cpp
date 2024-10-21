#include <Core/constants.hpp>
#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/icons.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/string_functions.hpp>
#include <Core/theme.hpp>
#include <Graphics/imgui.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/properties_window.hpp>
#include <imfilebrowser.h>
#include <imgui.h>
#include <imgui_internal.h>


namespace Engine
{
	Map<Refl::Struct*, void (*)(class ImGuiObjectProperties*, void*, Refl::Struct*, bool)> special_class_properties_renderers;
	static Map<size_t, ImGuiObjectProperties::PropertyRenderer> m_renderers;

	static bool render_struct_properties(ImGuiObjectProperties*, void* object, class Refl::Struct* struct_class,
										 bool editable = true, bool is_in_table = false);

	static FORCE_INLINE float get_column_width(ImGuiTableColumn& column)
	{
		return column.WorkMaxX - column.WorkMinX;
	}

	static inline bool props_collapsing_header(const void* id, const char* header_text)
	{
		return ImGuiObjectProperties::collapsing_header(id, "%s", header_text);
	}

	static FORCE_INLINE void begin_prop_table()
	{
		ImGui::BeginTable("##PropTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner);
		auto width = ImGui::GetContentRegionAvail().x;
		ImGui::TableSetupColumn("##Column1", ImGuiTableColumnFlags_WidthStretch, width * 0.45);
		ImGui::TableSetupColumn("##Column2", ImGuiTableColumnFlags_WidthStretch, width * 0.45);
		ImGui::TableSetupColumn("##Column3", ImGuiTableColumnFlags_WidthStretch, width * 0.1);
	}

	static FORCE_INLINE void end_prop_table()
	{
		ImGui::EndTable();
	}

	static FORCE_INLINE void push_props_id(const void* object, Property* prop)
	{
		ImGui::PushID(object);
		ImGui::PushID(prop);
	}

	static FORCE_INLINE void pop_props_id()
	{
		ImGui::PopID();
		ImGui::PopID();
	}

	static FORCE_INLINE void render_prop_name(Property* prop)
	{
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", prop->name().c_str());
		ImGui::TableSetColumnIndex(1);

		auto& desc = prop->description();

		if (!desc.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("%s", desc.c_str());
		}
	}

	static bool render_property(ImGuiObjectProperties*, void* object, Property* prop, bool can_edit);

	template<typename Type, PropertyType property_type = PropertyType::Undefined>
	static bool render_prop_internal(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit,
	                                 bool (*callback)(ImGuiObjectProperties* window, void*, Property*, Type&, bool can_edit))
	{
		render_prop_name(prop);

		PropertyValue result = prop->property_value(object);
		if (result.has_value())
		{
			Type value = result.cast<Type>();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (callback(window, object, prop, value, can_edit) && can_edit)
			{
				if constexpr (property_type == PropertyType::Undefined)
				{
					result = value;
				}
				else
				{
					result = PropertyValue(value, property_type);
				}
				prop->property_value(object, result);
				return true;
			}
		}
		return false;
	}


#define input_text_flags() (ImGuiInputTextFlags_EnterReturnsTrue | (editable ? 0 : ImGuiInputTextFlags_ReadOnly))
#define edit_f(type) [](ImGuiObjectProperties * window, void* object, Property* prop, type& value, bool editable) -> bool

	struct EnumRenderingUserdata {
		EnumerateType value;
		const Refl::Enum::Entry* current_entry;
		Refl::Enum* enum_class;
	};


	static const char* enum_element_name(void* userdata, int index)
	{
		EnumRenderingUserdata* data = reinterpret_cast<EnumRenderingUserdata*>(userdata);
		if (index == -1)
		{
			static thread_local char buffer[255];
			sprintf(buffer, "Undefined value <%d>", data->value);
			return buffer;
		}
		return data->enum_class->entries()[index].name.c_str();
	}

	static FORCE_INLINE int convert_to_imgui_index(Index index)
	{
		if (index == Constants::index_none)
			return -1;
		return static_cast<int>(index);
	}

	static bool render_enum_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
	{
		PropertyValue value = prop->property_value(object);
		if (!value.has_value())
			return false;

		EnumRenderingUserdata userdata;
		userdata.enum_class = prop->enum_instance();
		if (!userdata.enum_class)
			return false;


		EnumerateType current     = value.cast<EnumerateType>();
		const auto* current_entry = userdata.enum_class->entry(current);

		int index           = convert_to_imgui_index(userdata.enum_class->index_of(current));
		const auto& entries = userdata.enum_class->entries();

		render_prop_name(prop);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

		if (ImGui::Combo("##ComboValue", &index, enum_element_name, &userdata, entries.size()) && can_edit)
		{
			current_entry = &entries[index];
			value         = PropertyValue(current_entry->value, PropertyType::Enum);
			prop->property_value(object, value);
			return true;
		}

		return false;
	}

	static bool render_object_property(ImGuiObjectProperties* window, Object* object, Property* prop, bool can_edit)
	{
		PropertyValue value = prop->property_value(object);

		if (value.has_value())
		{
			object = value.cast<Object*>();

			if (object)
			{
				auto* struct_class = object->class_instance();
				if (props_collapsing_header(prop, prop->name().c_str()))
				{
					push_props_id(object, prop);
					ImGui::Indent(Settings::ed_collapsing_indent);
					render_struct_properties(window, object, struct_class, can_edit, true);
					ImGui::Unindent(Settings::ed_collapsing_indent);
					pop_props_id();
				}
			}
		}

		return false;
	}

	static bool render_object_reference_internal(ImGuiObjectProperties* window, void* object, Property* prop, Object*& value,
	                                             bool can_edit)
	{
		auto* self       = prop->struct_instance();
		const float size = ImGui::GetFrameHeight();

		bool changed = false;

		{
			ImGui::TableSetColumnIndex(1);

			ImGui::PushID("##Image");
			ImGui::Image(Icons::find_imgui_icon(value), {100, 100});

			if (value && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("%s", value->full_name().c_str());
			}

			if (can_edit && ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
				if (payload)
				{
					IM_ASSERT(payload->DataSize == sizeof(Object*));

					Object* new_object = *reinterpret_cast<Object**>(payload->Data);

					if (new_object->class_instance()->is_a(self))
					{
						value   = new_object;
						changed = true;
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopID();

			if (can_edit)
			{
				ImGui::TableSetColumnIndex(2);
				if (ImGui::ImageButton(ImTextureID(Icons::icon(Icons::IconType::Rotate), EditorResources::default_sampler),
				                       {size, size}))
				{
					value   = nullptr;
					changed = true;
				}
			}
		}
		return changed;
	}

	static bool render_object_reference(ImGuiObjectProperties* window, Object* object, Property* prop, bool can_edit)
	{
		return render_prop_internal<Object*, PropertyType::ObjectReference>(window, object, prop, can_edit,
		                                                                    render_object_reference_internal);
	}


	static bool render_struct_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
	{
		PropertyValue value = prop->property_value(object);

		bool is_changed = false;

		if (value.has_value())
		{
			void* struct_object = value.cast<void*>();
			auto* struct_class  = prop->struct_instance();
			if (props_collapsing_header(prop, prop->name().c_str()))
			{
				push_props_id(object, prop);
				ImGui::Indent(Settings::ed_collapsing_indent);
				is_changed = render_struct_properties(window, struct_object, struct_class, can_edit, true);
				ImGui::Unindent(Settings::ed_collapsing_indent);
				pop_props_id();

				if (is_changed)
				{
					prop->on_prop_changed(object);
				}
			}
		}

		return is_changed;
	}

	static bool render_array_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
	{
		PropertyValue value = prop->property_value(object);
		if (!value.has_value())
			return false;

		ImGui::TableSetColumnIndex(2);
		ArrayPropertyInterface* interface = reinterpret_cast<ArrayPropertyInterface*>(prop);
		const float size                  = ImGui::GetFrameHeight();

		bool is_changed = false;

		if (can_edit && ImGui::Button("+", {size, size}))
		{
			interface->emplace_back(object);
			is_changed = true;
		}

		if (props_collapsing_header(prop, prop->name().c_str()))
		{
			ImGui::Indent(Settings::ed_collapsing_indent);
			Property* element_property = interface->element_type();

			size_t count = interface->elements_count(object);
			auto name    = element_property->name();
			static Vector<Name> names;


			for (size_t i = 0; i < count;)
			{
				window->setup_next_row();
				ImGui::TableSetColumnIndex(2);

				ImGui::PushID(i);

				if (can_edit && ImGui::Button("-", {size, size}))
				{
					interface->erase(object, i);
					--count;
					ImGui::PopID();
					continue;
				}

				void* array_object = interface->at(object, i);
				element_property->name(interface->element_name(object, i));
				if (render_property(window, array_object, element_property, true))
					is_changed = true;
				++i;
				ImGui::PopID();
			}

			element_property->name(name);
			ImGui::Unindent(Settings::ed_collapsing_indent);
		}

		if (is_changed)
		{
			prop->on_prop_changed(object);
		}
		return is_changed;
	}

	static bool render_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
	{
		can_edit = can_edit && !prop->is_const();
		push_props_id(object, prop);

		bool is_changed = false;

		auto renderer = m_renderers.find(prop->type_id());

		if (renderer != m_renderers.end())
		{
			is_changed = (*renderer).second(window, object, prop, can_edit);
		}
		else
		{
			switch (prop->type())
			{
				case PropertyType::Enum:
					is_changed = render_enum_property(window, object, prop, can_edit);
					break;
				case PropertyType::Object:
					is_changed = render_object_property(window, reinterpret_cast<Object*>(object), prop, can_edit);
					break;
				case PropertyType::ObjectReference:
					is_changed = render_object_reference(window, reinterpret_cast<Object*>(object), prop, can_edit);
					break;
				case PropertyType::Struct:
					is_changed = render_struct_property(window, object, prop, can_edit);
					break;

				case PropertyType::Array:
					is_changed = render_array_property(window, object, prop, can_edit);
					break;
				default:
					break;
			}
		}

		pop_props_id();
		return is_changed;
	}

	static bool render_struct_properties(ImGuiObjectProperties* window, void* object, Refl::Struct* struct_class, bool editable,
	                                     bool is_in_table)
	{
		if (!is_in_table)
			begin_prop_table();

		for (Refl::Struct* self = struct_class; self; self = self->parent())
		{
			auto it = special_class_properties_renderers.find(self);

			if (it != special_class_properties_renderers.end())
			{
				it->second(window, object, struct_class, editable);
			}
		}

		bool has_changed_props = false;

		for (auto& [group, props] : window->properties_map(struct_class))
		{
			bool open = true;

			if (group != Name::none)
			{
				window->setup_next_row();
				++window->row_index;

				open = props_collapsing_header(group.c_str(), group.c_str());
				ImGui::Indent(Settings::ed_collapsing_indent);
			}

			if (open)
			{
				for (auto& prop : props)
				{
					if (!prop->is_hidden())
					{
						window->setup_next_row();
						if (render_property(window, object, prop, editable))
							has_changed_props = true;
					}
				}
			}

			if (group != Name::none)
			{
				ImGui::Unindent(Settings::ed_collapsing_indent);
			}
		}

		if (!is_in_table)
			end_prop_table();

		return has_changed_props;
	}

	ImGuiObjectProperties::ImGuiObjectProperties() : m_object(nullptr)
	{
		m_destroy_id = GarbageCollector::on_destroy.push([this](Object* object) {
			if (object == m_object)
			{
				m_object = nullptr;
			}
		});
	}

	ImGuiObjectProperties::~ImGuiObjectProperties()
	{
		GarbageCollector::on_destroy.remove(m_destroy_id);
	}

	bool ImGuiObjectProperties::render(RenderViewport* viewport)
	{
		bool open = true;
		ImGui::Begin(name(), closable ? &open : nullptr);

		on_begin_render.trigger(m_object);

		if (m_object)
		{
			ImGui::Text("editor/Object: %s"_localized, m_object->name().to_string().c_str());
			ImGui::Text("editor/Class: %s"_localized, m_object->class_instance()->full_name().c_str());
			if (ImGui::Button("editor/Apply changes"_localized))
			{
				m_object->apply_changes();
			}
			ImGui::Separator();
			row_index = 0;
			::Engine::render_struct_properties(this, m_object, m_object->class_instance(), true, false);
		}
		ImGui::End();

		return open;
	}

	Refl::Struct* ImGuiObjectProperties::struct_instance() const
	{
		return m_object ? m_object->class_instance() : nullptr;
	}

	Object* ImGuiObjectProperties::object() const
	{
		return m_object;
	}


	ImGuiObjectProperties::PropertiesMap& ImGuiObjectProperties::build_props_map(Refl::Struct* self)
	{
		auto& map = m_properties[self];
		map.clear();

		for (; self; self = self->parent())
		{
			for (auto& prop : self->properties())
			{
				map[prop->group()].push_back(prop);
			}
		}

		return map;
	}


	ImGuiObjectProperties& ImGuiObjectProperties::update(Object* object)
	{
		m_object = object;
		build_props_map(struct_instance());
		return *this;
	}

	const ImGuiObjectProperties::PropertiesMap& ImGuiObjectProperties::properties_map(Refl::Struct* self)
	{
		auto& map = m_properties[self];

		static auto has_props = [](Refl::Struct* self) -> bool {
			while (self)
			{
				if (!self->properties().empty())
					return true;
				self = self->parent();
			}
			return false;
		};

		if (map.empty() && has_props(self))
		{
			return build_props_map(self);
		}

		return map;
	}

	ImGuiObjectProperties& ImGuiObjectProperties::render_struct_properties(void* object, Refl::Struct* struct_class,
	                                                                       bool editable)
	{
		::Engine::render_struct_properties(this, object, struct_class, editable, true);
		return *this;
	}

	ImGuiObjectProperties& ImGuiObjectProperties::setup_next_row()
	{
		ImGui::TableNextRow();
		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
		                       !(row_index % 2) ? ImGui::ColorConvertFloat4ToU32(EditorTheme::table_row_color1)
		                                        : ImGui::ColorConvertFloat4ToU32(EditorTheme::table_row_color2));
		++row_index;
		return *this;
	}

	bool ImGuiObjectProperties::collapsing_header(const void* id, const char* format, ...)
	{
		::ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiTable* table     = ImGui::GetCurrentContext()->CurrentTable;

		ImGui::TableSetColumnIndex(0);
		ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
		color.w      = 1.0f;
		auto padding = ImGui::GetStyle().CellPadding;
		float indent = window->DC.Indent.x;

		auto min_pos = ImGui::GetCursorScreenPos() - padding - ImVec2(indent, 0.f);
		auto max_pos = min_pos + ImVec2(window->ParentWorkRect.GetWidth() + indent, ImGui::GetFrameHeight()) + padding * 2.f;

		ImGui::TablePushBackgroundChannel();
		ImGui::GetWindowDrawList()->AddRectFilled(min_pos, max_pos, ImGui::ColorConvertFloat4ToU32(color));
		ImGui::TablePopBackgroundChannel();

		auto clip_rect        = window->ClipRect;
		auto parent_work_rect = window->ParentWorkRect;

		max_pos.x -= get_column_width(table->Columns[2]) + (padding.x * 1.f) + indent;
		window->ClipRect.Max.x += (max_pos.x - min_pos.x) - clip_rect.GetWidth();
		window->ParentWorkRect.Max = max_pos;

		va_list args;
		va_start(args, format);
		bool result =
		        ImGui::TreeNodeExV(id, ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_CollapsingHeader, format, args);
		va_end(args);

		window->ClipRect       = clip_rect;
		window->ParentWorkRect = parent_work_rect;
		return result;
	}

	const char* ImGuiObjectProperties::name() const
	{
		return static_name();
	}

	const char* ImGuiObjectProperties::static_name()
	{
		return "editor/Properties Title"_localized;
	}

	void ImGuiObjectProperties::register_prop_renderer(size_t type_id, const PropertyRenderer& renderer)
	{
		m_renderers[type_id] = renderer;
	}

	template<typename T, ImGuiDataType type, int count, bool maybe_color>
	static bool render_primitive(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
	{
		render_prop_name(prop);

		T* value = reinterpret_cast<T*>(prop->prop_address(object));
		if (value)
		{
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			int_t flags = ImGuiInputTextFlags_EnterReturnsTrue | (can_edit ? 0 : ImGuiInputTextFlags_ReadOnly);

			if constexpr (maybe_color)
			{
				if (prop->is_color())
				{
					auto func = count == 3 ? &ImGui::ColorEdit3 : &ImGui::ColorEdit4;

					if (func("##value", reinterpret_cast<float*>(value), 0))
					{
						prop->on_prop_changed(object);
						return true;
					}
					return false;
				}
			}


			if (ImGui::InputScalarN("##value", type, value, count, nullptr, nullptr, nullptr, flags) && can_edit)
			{
				prop->on_prop_changed(object);
				return true;
			}
		}
		return false;
	}

	template<int count>
	static bool render_bool_primitive(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
	{
		render_prop_name(prop);

		bool* value = reinterpret_cast<bool*>(prop->prop_address(object));
		if (value)
		{
			static char name[] = "##value0";
			bool edited        = false;

			for (int i = 0; i < count; ++i)
			{
				name[7]  = '0' + i;
				bool tmp = *value;

				if (ImGui::Checkbox(name, &tmp) && can_edit)
				{
					edited = true;
					*value = tmp;
					prop->on_prop_changed(object);
				}

				++value;
			}

			return edited;
		}
		return false;
	}

	template<typename T, ImGuiDataType type, int count>
	static void register_primitive_renderer()
	{
		constexpr bool maybe_color = std::is_same_v<T, Vector3D> || std::is_same_v<T, Vector4D>;
		ImGuiObjectProperties::register_prop_renderer<T>(render_primitive<T, type, count, maybe_color>);
	}

	static void on_preinit()
	{
		using T = ImGuiObjectProperties;

		T::register_prop_renderer<bool>(render_bool_primitive<1>);
		register_primitive_renderer<int8_t, ImGuiDataType_S8, 1>();
		register_primitive_renderer<uint8_t, ImGuiDataType_U8, 1>();
		register_primitive_renderer<int16_t, ImGuiDataType_S16, 1>();
		register_primitive_renderer<uint16_t, ImGuiDataType_U16, 1>();
		register_primitive_renderer<int32_t, ImGuiDataType_S32, 1>();
		register_primitive_renderer<uint32_t, ImGuiDataType_U32, 1>();
		register_primitive_renderer<int64_t, ImGuiDataType_S64, 1>();
		register_primitive_renderer<uint64_t, ImGuiDataType_U64, 1>();
		register_primitive_renderer<float, ImGuiDataType_Float, 1>();
		register_primitive_renderer<double, ImGuiDataType_Double, 1>();

		// Register glm types
		T::register_prop_renderer<glm::bvec1>(render_bool_primitive<1>);
		T::register_prop_renderer<glm::bvec2>(render_bool_primitive<2>);
		T::register_prop_renderer<glm::bvec3>(render_bool_primitive<3>);
		T::register_prop_renderer<glm::bvec4>(render_bool_primitive<4>);

		register_primitive_renderer<glm::i8vec1, ImGuiDataType_S8, 1>();
		register_primitive_renderer<glm::i8vec2, ImGuiDataType_S8, 2>();
		register_primitive_renderer<glm::i8vec3, ImGuiDataType_S8, 3>();
		register_primitive_renderer<glm::i8vec4, ImGuiDataType_S8, 4>();

		register_primitive_renderer<glm::u8vec1, ImGuiDataType_U8, 1>();
		register_primitive_renderer<glm::u8vec2, ImGuiDataType_U8, 2>();
		register_primitive_renderer<glm::u8vec3, ImGuiDataType_U8, 3>();
		register_primitive_renderer<glm::u8vec4, ImGuiDataType_U8, 4>();

		register_primitive_renderer<glm::i16vec1, ImGuiDataType_S16, 1>();
		register_primitive_renderer<glm::i16vec2, ImGuiDataType_S16, 2>();
		register_primitive_renderer<glm::i16vec3, ImGuiDataType_S16, 3>();
		register_primitive_renderer<glm::i16vec4, ImGuiDataType_S16, 4>();

		register_primitive_renderer<glm::u16vec1, ImGuiDataType_U16, 1>();
		register_primitive_renderer<glm::u16vec2, ImGuiDataType_U16, 2>();
		register_primitive_renderer<glm::u16vec3, ImGuiDataType_U16, 3>();
		register_primitive_renderer<glm::u16vec4, ImGuiDataType_U16, 4>();

		register_primitive_renderer<glm::i32vec1, ImGuiDataType_S32, 1>();
		register_primitive_renderer<glm::i32vec2, ImGuiDataType_S32, 2>();
		register_primitive_renderer<glm::i32vec3, ImGuiDataType_S32, 3>();
		register_primitive_renderer<glm::i32vec4, ImGuiDataType_S32, 4>();

		register_primitive_renderer<glm::u32vec1, ImGuiDataType_U32, 1>();
		register_primitive_renderer<glm::u32vec2, ImGuiDataType_U32, 2>();
		register_primitive_renderer<glm::u32vec3, ImGuiDataType_U32, 3>();
		register_primitive_renderer<glm::u32vec4, ImGuiDataType_U32, 4>();

		register_primitive_renderer<glm::i64vec1, ImGuiDataType_S64, 1>();
		register_primitive_renderer<glm::i64vec2, ImGuiDataType_S64, 2>();
		register_primitive_renderer<glm::i64vec3, ImGuiDataType_S64, 3>();
		register_primitive_renderer<glm::i64vec4, ImGuiDataType_S64, 4>();

		register_primitive_renderer<glm::u64vec1, ImGuiDataType_U64, 1>();
		register_primitive_renderer<glm::u64vec2, ImGuiDataType_U64, 2>();
		register_primitive_renderer<glm::u64vec3, ImGuiDataType_U64, 3>();
		register_primitive_renderer<glm::u64vec4, ImGuiDataType_U64, 4>();

		register_primitive_renderer<glm::vec1, ImGuiDataType_Float, 1>();
		register_primitive_renderer<glm::vec2, ImGuiDataType_Float, 2>();
		register_primitive_renderer<glm::vec3, ImGuiDataType_Float, 3>();
		register_primitive_renderer<glm::vec4, ImGuiDataType_Float, 4>();

		register_primitive_renderer<glm::dvec1, ImGuiDataType_Double, 1>();
		register_primitive_renderer<glm::dvec2, ImGuiDataType_Double, 2>();
		register_primitive_renderer<glm::dvec3, ImGuiDataType_Double, 3>();
		register_primitive_renderer<glm::dvec4, ImGuiDataType_Double, 4>();

		T::register_prop_renderer<String>([](ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit) -> bool {
			render_prop_name(prop);

			if (String* value = reinterpret_cast<String*>(prop->prop_address(object)))
			{
				auto flags = ImGuiInputTextFlags_EnterReturnsTrue | (can_edit ? 0 : ImGuiInputTextFlags_ReadOnly);

				if (ImGui::InputText("##Value", *value, flags))
				{
					prop->on_prop_changed(object);
					return true;
				}
			}
			return false;
		});

		T::register_prop_renderer<Path>([](ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit) -> bool {
			render_prop_name(prop);

			if (Path* value = reinterpret_cast<Path*>(prop->prop_address(object)))
			{
				ImGuiWindow* imgui_window = ImGuiWindow::current();

				const char* text = value->empty() ? "None" : value->c_str();
				if (ImGui::Selectable(text) && can_edit)
				{
					Function<void(const Path&)> callback = [object, prop](const Path& path) {
						prop->property_value(object, path);
					};
					imgui_window->widgets_list.create<ImGuiOpenFile>()->on_select.push(callback);
				}
			}
			return false;
		});

		T::register_prop_renderer<Name>([](ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit) -> bool {
			render_prop_name(prop);

			if (Name* value = reinterpret_cast<Name*>(prop->prop_address(object)))
			{
				ImGui::Text("%s", value->c_str());
			}
			return false;
		});
	}

	static PreInitializeController pre_init(on_preinit);
}// namespace Engine
