#include <Core/constants.hpp>
#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Core/exception.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/icons.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/theme.hpp>
#include <Graphics/imgui.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/properties_window.hpp>
#include <imfilebrowser.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>


namespace Engine
{
	Map<Refl::Struct*, void (*)(ImGuiObjectProperties*, void*, Refl::Struct*, bool)> special_class_properties_renderers;
	static Map<const Refl::Object::ReflClassInfo*, ImGuiObjectProperties::PropertyRenderer> m_renderers;

	template<typename T>
	static FORCE_INLINE T* prop_cast(Refl::Property* prop)
	{
		return Refl::Object::instance_cast<T>(prop);
	}

	template<typename T>
	static FORCE_INLINE T* prop_cast_checked(Refl::Property* prop)
	{
		auto res = prop_cast<T>(prop);
		trinex_always_check(res, "Failed to cast property");
		return res;
	}

	static FORCE_INLINE void push_props_id(const void* object, Refl::Property* prop)
	{
		ImGui::PushID(object);
		ImGui::PushID(prop);
	}

	static FORCE_INLINE void pop_props_id()
	{
		ImGui::PopID();
		ImGui::PopID();
	}

	static FORCE_INLINE void render_prop_name(Refl::Property* prop)
	{
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", prop->display_name().c_str());
		ImGui::TableSetColumnIndex(1);

		auto& tooltip = prop->tooltip();

		if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("%s", tooltip.c_str());
		}
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

			ImGui::BeginTable("##PropTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner);
			auto width = ImGui::GetContentRegionAvail().x;
			ImGui::TableSetupColumn("##Column1", ImGuiTableColumnFlags_WidthStretch, width * 0.45);
			ImGui::TableSetupColumn("##Column2", ImGuiTableColumnFlags_WidthStretch, width * 0.45);
			ImGui::TableSetupColumn("##Column3", ImGuiTableColumnFlags_WidthStretch, width * 0.1);

			render_struct_properties(m_object, m_object->class_instance(), false);

			ImGui::EndTable();
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

	bool ImGuiObjectProperties::render_property(void* object, Refl::Property* prop, bool read_only)
	{
		read_only = read_only || prop->is_read_only();
		push_props_id(object, prop);

		bool is_changed = false;

		auto renderer = m_renderers.find(prop->refl_class_info());

		if (renderer != m_renderers.end())
		{
			is_changed = (*renderer).second(this, object, prop, read_only);
		}

		pop_props_id();
		return is_changed;
	}

	bool ImGuiObjectProperties::render_struct_properties(void* object, Refl::Struct* struct_class, bool read_only)
	{
		bool has_changed_props = false;

		for (auto& [group, props] : properties_map(struct_class))
		{
			bool open = true;

			if (!group.empty())
			{
				ImGui::TableNextRow();
				open = collapsing_header(group.c_str(), "%s", group.c_str());
				ImGui::Indent(Settings::ed_collapsing_indent);
			}

			if (open)
			{
				for (auto& prop : props)
				{
					if (!prop->is_hidden())
					{
						ImGui::TableNextRow();
						if (render_property(object, prop, read_only))
							has_changed_props = true;
					}
				}
			}

			if (!group.empty())
			{
				ImGui::Unindent(Settings::ed_collapsing_indent);
			}
		}

		return has_changed_props;
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


		max_pos.x -= (table->Columns[2].WorkMaxX - table->Columns[2].WorkMinX) + (padding.x * 1.f) + indent;
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

	void ImGuiObjectProperties::register_prop_renderer(const Refl::Object::ReflClassInfo* refl_class,
													   const PropertyRenderer& renderer)
	{
		m_renderers[refl_class] = renderer;
	}


	//////////////////////// PROPERTY RENDERERS ////////////////////////

	static bool render_boolean_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop, bool read_only)
	{
		render_prop_name(prop);
		bool* value_address = prop->address_as<bool>(context);
		bool value          = *value_address;

		if (ImGui::Checkbox("##value", &value) && !read_only)
		{
			(*value_address) = value;
			prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
			return true;
		}

		return false;
	}

	static bool render_scalar_property(void* context, Refl::Property* prop, ImGuiDataType type, int_t components, bool read_only)
	{
		ImGuiInputTextFlags flags = (read_only ? ImGuiInputTextFlags_ReadOnly : 0) | ImGuiInputTextFlags_EnterReturnsTrue;
		void* address             = prop->address(context);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::InputScalarN("##value", type, address, components, nullptr, nullptr, nullptr, flags))
		{
			prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
			return true;
		}
		return false;
	}

	static bool render_integer_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop = prop_cast_checked<Refl::IntegerProperty>(prop_base);
		render_prop_name(prop_base);
		auto type = ((prop->size() - 1) * 2) + static_cast<ImGuiDataType>(prop->is_unsigned());
		return render_scalar_property(context, prop, type, 1, read_only);
	}

	static bool render_float_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop, bool read_only)
	{
		render_prop_name(prop);
		auto type = prop->size() == 4 ? ImGuiDataType_Float : ImGuiDataType_Double;
		return render_scalar_property(context, prop, type, 1, read_only);
	}

	static bool render_vector_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop    = prop_cast_checked<Refl::VectorProperty>(prop_base);
		auto element = prop->element_property();

		auto render_scalar = [&](ImGuiDataType type) -> bool {
			render_prop_name(prop);
			bool is_changed = render_scalar_property(prop->address(context), element, type, prop->length(),
													 read_only || element->is_read_only());
			if (is_changed)
				prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));

			return is_changed;
		};

		if (element->is_a<Refl::BooleanProperty>())
		{
			render_prop_name(prop);
			bool* value_address = prop->address_as<bool>(context);

			char name[] = "##value0";
			for (size_t i = 0, count = prop->length(); i < count; ++i)
			{
				bool value = *(value_address + i);
				name[7]    = '0' + i;
				if (ImGui::Checkbox(name, &value) && !read_only)
				{
					element->on_property_changed(
							Refl::PropertyChangedEvent(value_address, Refl::PropertyChangeType::value_set, element));
					(*(value_address)) = value;
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));
					return true;
				}
			}
		}
		else if (auto integer = prop_cast<Refl::IntegerProperty>(element))
		{
			auto type = ((integer->size() - 1) * 2) + static_cast<ImGuiDataType>(integer->is_unsigned());
			return render_scalar(type);
		}
		else if (element->is_a<Refl::FloatProperty>())
		{
			auto type = element->size() == 4 ? ImGuiDataType_Float : ImGuiDataType_Double;
			return render_scalar(type);
		}
		else
		{
		}
		return false;
	}

	static bool render_enum_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop      = prop_cast_checked<Refl::EnumProperty>(prop_base);
		auto enum_inst = prop->enum_instance();

		render_prop_name(prop);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

		EnumerateType value       = prop->value(context);
		const auto* current_entry = enum_inst->entry(value);

		bool is_changed = false;

		if (ImGui::BeginCombo("##ComboValue", current_entry->name.c_str()))
		{
			for (auto& entry : enum_inst->entries())
			{
				bool is_selected = entry.value == value;

				if (ImGui::Selectable(entry.name.c_str(), is_selected) && !read_only)
				{
					prop->value(context, entry.value);
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
					is_changed = true;
				}
			}

			ImGui::EndCombo();
		}

		return is_changed;
	}

	static bool render_string_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop, bool read_only)
	{
		render_prop_name(prop);

		if (String* value = prop->address_as<String>(context))
		{
			auto flags = ImGuiInputTextFlags_EnterReturnsTrue | (read_only ? ImGuiInputTextFlags_ReadOnly : 0);

			if (ImGui::InputText("##Value", *value, flags))
			{
				prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
				return true;
			}
		}
		return false;
	}

	static bool render_name_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop, bool read_only)
	{
		render_prop_name(prop);

		if (Name* value = prop->address_as<Name>(context))
		{
			ImGui::Text("%s", value->c_str());
		}

		return false;
	}

	static bool render_path_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop, bool read_only)
	{
		render_prop_name(prop);

		if (Path* value = prop->address_as<Path>(context))
		{
			ImGuiWindow* imgui_window = ImGuiWindow::current();
			const char* text          = value->empty() ? "None" : value->c_str();

			if (ImGui::Selectable(text) && !read_only)
			{
				Function<void(const Path&)> callback = [context, prop, value](const Path& path) {
					*value = path;
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
				};

				imgui_window->widgets_list.create<ImGuiOpenFile>()->on_select.push(callback);
			}
		}
		return false;
	}

	static bool render_struct_property_internal(ImGuiObjectProperties* window, void* context, void* struct_address,
												Refl::Property* prop, Refl::Struct* struct_instance, bool read_only)
	{
		bool is_changed = false;

		if (window->collapsing_header(prop, "%s", prop->display_name().c_str()))
		{
			push_props_id(struct_address, prop);
			ImGui::Indent(Settings::ed_collapsing_indent);
			is_changed = window->render_struct_properties(struct_address, struct_instance, read_only);
			ImGui::Unindent(Settings::ed_collapsing_indent);
			pop_props_id();

			if (is_changed)
			{
				Refl::PropertyChangedEvent event(context, Refl::PropertyChangeType::member_change, prop);
				prop->on_property_changed(event);
			}
		}
		return is_changed;
	}

	static bool render_object_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop_base, bool read_only)
	{
		Refl::ObjectProperty* prop = prop_cast_checked<Refl::ObjectProperty>(prop_base);
		auto object                = prop->object(context);

		if (prop->is_composite())
		{
			return render_struct_property_internal(window, context, prop->object(context), prop, object->class_instance(),
												   read_only || prop->is_read_only());
		}
		else
		{
			auto* self       = object->class_instance();
			const float size = ImGui::GetFrameHeight();
			auto object      = prop->object(context);

			bool changed = false;

			ImGui::TableSetColumnIndex(1);

			ImGui::PushID("##Image");
			ImGui::Image(Icons::find_imgui_icon(object), {100, 100});

			if (object && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("%s", object->full_name().c_str());
			}

			if (!read_only && ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
				if (payload)
				{
					IM_ASSERT(payload->DataSize == sizeof(Object*));

					Object* new_object = *reinterpret_cast<Object**>(payload->Data);

					if (new_object->class_instance()->is_a(self))
					{
						object  = new_object;
						changed = true;
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopID();

			if (!read_only)
			{
				ImGui::TableSetColumnIndex(2);
				if (ImGui::ImageButton(ImTextureID(Icons::icon(Icons::IconType::Rotate), EditorResources::default_sampler),
									   {size, size}))
				{
					object  = nullptr;
					changed = true;
				}
			}

			return changed;
		}

		return false;
	}

	static bool render_struct_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop_base, bool read_only)
	{
		Refl::StructProperty* prop = prop_cast_checked<Refl::StructProperty>(prop_base);
		return render_struct_property_internal(window, context, prop->address(context), prop, prop->struct_instance(),
											   read_only || prop->is_read_only());
	}

	static bool render_array_property(ImGuiObjectProperties* window, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop = prop_cast_checked<Refl::ArrayProperty>(prop_base);

		ImGui::TableSetColumnIndex(2);
		const float size = ImGui::GetFrameHeight();
		bool is_changed  = false;

		if (!read_only && ImGui::Button("+", {size, size}))
		{
			prop->emplace_back(context);
			prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::array_add, prop));
			is_changed = true;
		}

		if (window->collapsing_header(prop, "%s", prop->display_name().c_str()))
		{
			ImGui::Indent(Settings::ed_collapsing_indent);
			Refl::Property* element_prop = prop->element_property();

			size_t count = prop->length(context);

			for (size_t i = 0; i < count; ++i)
			{
				ImGui::TableNextRow();
				ImGui::PushID(i);

				ImGui::TableSetColumnIndex(2);

				if (!read_only && ImGui::Button("-", {size, size}))
				{
					prop->erase(context, i);
					--count;
					ImGui::PopID();
					continue;
				}

				void* array_object = prop->at(context, i);

				if (window->render_property(array_object, element_prop, read_only || element_prop->is_read_only()))
				{
					is_changed = true;
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));
				}

				ImGui::PopID();
			}

			ImGui::Unindent(Settings::ed_collapsing_indent);
		}

		return is_changed;
	}

	static void on_preinit()
	{
		using T = ImGuiObjectProperties;

		T::register_prop_renderer<Refl::BooleanProperty>(render_boolean_property);
		T::register_prop_renderer<Refl::IntegerProperty>(render_integer_property);
		T::register_prop_renderer<Refl::FloatProperty>(render_float_property);
		T::register_prop_renderer<Refl::VectorProperty>(render_vector_property);
		T::register_prop_renderer<Refl::EnumProperty>(render_enum_property);
		T::register_prop_renderer<Refl::StringProperty>(render_string_property);
		T::register_prop_renderer<Refl::NameProperty>(render_name_property);
		T::register_prop_renderer<Refl::PathProperty>(render_path_property);
		T::register_prop_renderer<Refl::StructProperty>(render_struct_property);
		T::register_prop_renderer<Refl::ObjectProperty>(render_object_property);
		T::register_prop_renderer<Refl::ArrayProperty>(render_array_property);
	}

	static PreInitializeController pre_init(on_preinit);
}// namespace Engine
