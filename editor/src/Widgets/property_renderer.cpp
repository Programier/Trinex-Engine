#include <Core/constants.hpp>
#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/icons.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/object.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/theme.hpp>
#include <Graphics/imgui.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/property_renderer.hpp>
#include <imfilebrowser.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	static Map<const Refl::ClassInfo*, PropertyRenderer::RendererFunc> m_renderers;

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

	struct ScopedPropID {
		ScopedPropID(const void* object, Refl::Property* prop)
		{
			push_props_id(object, prop);
		}

		~ScopedPropID()
		{
			pop_props_id();
		}
	};

	PropertyRenderer::PropertyRenderer() : m_object(nullptr), m_is_property_skipped(false)
	{
		m_destroy_id = GarbageCollector::on_destroy.push([this](Object* object) {
			if (object == m_object)
			{
				m_object = nullptr;
			}
		});
	}

	PropertyRenderer::~PropertyRenderer()
	{
		GarbageCollector::on_destroy.remove(m_destroy_id);
	}

	bool PropertyRenderer::render(RenderViewport* viewport)
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

			m_is_property_skipped = false;
			m_next_prop_name      = "";

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

	Refl::Struct* PropertyRenderer::struct_instance() const
	{
		return m_object ? m_object->class_instance() : nullptr;
	}

	Object* PropertyRenderer::object() const
	{
		return m_object;
	}

	PropertyRenderer::PropertiesMap& PropertyRenderer::build_props_map(Refl::Struct* self)
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

	PropertyRenderer& PropertyRenderer::update(Object* object)
	{
		m_object = object;
		m_properties.clear();
		m_userdata.clear();
		build_props_map(struct_instance());
		return *this;
	}

	const PropertyRenderer::PropertiesMap& PropertyRenderer::properties_map(Refl::Struct* self)
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

	void PropertyRenderer::mark_property_skipped()
	{
		m_is_property_skipped = true;
	}

	bool PropertyRenderer::is_property_skipped() const
	{
		return m_is_property_skipped;
	}

	void PropertyRenderer::create_row()
	{
		if (m_is_property_skipped)
		{
			m_is_property_skipped = false;
		}
		else
		{
			ImGui::TableNextRow();
		}
	}

	void PropertyRenderer::next_prop_name(const String& name)
	{
		m_next_prop_name = name;
	}

	const String& PropertyRenderer::next_prop_name() const
	{
		return m_next_prop_name;
	}

	void PropertyRenderer::render_name(Refl::Property* prop)
	{
		ImGui::TableSetColumnIndex(0);
		if (m_next_prop_name.empty())
		{
			ImGui::Text("%s", prop->display_name().c_str());
		}
		else
		{
			ImGui::Text("%s", m_next_prop_name.c_str());
			m_next_prop_name.clear();
		}

		ImGui::TableSetColumnIndex(1);

		auto& tooltip = prop->tooltip();

		if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("%s", tooltip.c_str());
		}
	}

	bool PropertyRenderer::render_property(void* object, Refl::Property* prop, bool read_only, bool allow_script_call)
	{
		read_only = read_only || prop->is_read_only();
		ScopedPropID prop_id(object, prop);

		if (allow_script_call)
		{
			const ScriptFunction& specific_renderer = prop->renderer();

			if (specific_renderer.is_valid())
			{
				return ScriptContext::execute(specific_renderer, object, prop, this, read_only).bool_value();
			}
		}

		auto renderer = m_renderers.find(prop->refl_class_info());

		if (renderer != m_renderers.end())
		{
			return (*renderer).second(this, object, prop, read_only);
		}

		return false;
	}

	bool PropertyRenderer::render_struct_properties(void* object, Refl::Struct* struct_class, bool read_only)
	{
		bool has_changed_props = false;

		for (auto& [group, props] : properties_map(struct_class))
		{
			bool open = true;

			if (!group.empty())
			{
				create_row();
				open = collapsing_header(group.c_str(), "%s", group.c_str());
				ImGui::Indent();
			}

			if (open)
			{
				for (auto& prop : props)
				{
					if (!prop->is_hidden())
					{
						create_row();
						if (render_property(object, prop, read_only))
							has_changed_props = true;
					}
				}
			}

			if (!group.empty())
			{
				ImGui::Unindent();
			}
		}

		return has_changed_props;
	}

	bool PropertyRenderer::collapsing_header(Refl::Property* prop)
	{
		const char* name = m_next_prop_name.empty() ? prop->display_name().c_str() : m_next_prop_name.c_str();
		bool res         = collapsing_header(prop, "%s", name);

		if (!m_next_prop_name.empty())
			m_next_prop_name.clear();

		return res;
	}

	bool PropertyRenderer::collapsing_header(const void* id, const char* format, ...)
	{
		::ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiTable* table     = ImGui::GetCurrentContext()->CurrentTable;

		ImGui::TableSetColumnIndex(0);
		auto padding = ImGui::GetStyle().CellPadding;
		float indent = window->DC.Indent.x;

		auto min_pos = ImGui::GetCursorScreenPos() - padding - ImVec2(indent, 0.f);
		auto max_pos = min_pos + ImVec2(window->ParentWorkRect.GetWidth() + indent, ImGui::GetFrameHeight()) + padding * 2.f;

		auto clip_rect        = window->ClipRect;
		auto parent_work_rect = window->ParentWorkRect;

		max_pos.x -= (table->Columns[2].WorkMaxX - table->Columns[2].WorkMinX) + (padding.x * 1.f) + indent;
		window->ClipRect.Max.x += (max_pos.x - min_pos.x) - clip_rect.GetWidth();
		window->ParentWorkRect.Max = max_pos;

		constexpr auto flags =
				ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_NoAutoOpenOnLog;
		va_list args;
		va_start(args, format);
		bool result = ImGui::TreeNodeExV(id, flags, format, args);
		va_end(args);

		window->ClipRect       = clip_rect;
		window->ParentWorkRect = parent_work_rect;
		return result;
	}

	const char* PropertyRenderer::name() const
	{
		return static_name();
	}

	const char* PropertyRenderer::static_name()
	{
		return "editor/Properties Title"_localized;
	}

	void PropertyRenderer::register_prop_renderer(const Refl::ClassInfo* refl_class, const RendererFunc& renderer)
	{
		m_renderers[refl_class] = renderer;
	}

	//////////////////////// PROPERTY RENDERERS ////////////////////////

	static bool render_boolean_property(PropertyRenderer* renderer, void* context, Refl::Property* prop, bool read_only)
	{
		renderer->render_name(prop);
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
		ImGuiInputTextFlags flags = (read_only ? ImGuiInputTextFlags_ReadOnly : 0);
		void* address             = prop->address(context);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::InputScalarN("##value", type, address, components, nullptr, nullptr, nullptr, flags))
		{
			prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
			return true;
		}
		return false;
	}

	static ImGuiDataType find_imgui_data_type(Refl::IntegerProperty* prop)
	{
		return (std::countr_zero(prop->size()) * 2) + static_cast<ImGuiDataType>(prop->is_unsigned());
	}

	static ImGuiDataType find_imgui_data_type(Refl::FloatProperty* prop)
	{
		return prop->size() == 4 ? ImGuiDataType_Float : ImGuiDataType_Double;
	}

	static bool render_integer_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop = prop_cast_checked<Refl::IntegerProperty>(prop_base);
		renderer->render_name(prop_base);
		return render_scalar_property(context, prop, find_imgui_data_type(prop), 1, read_only);
	}

	static bool render_float_property(PropertyRenderer* renderer, void* context, Refl::Property* prop, bool read_only)
	{
		renderer->render_name(prop);
		auto float_prop = prop_cast_checked<Refl::FloatProperty>(prop);
		return render_scalar_property(context, prop, find_imgui_data_type(float_prop), 1, read_only);
	}

	static bool render_vector_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop    = prop_cast_checked<Refl::VectorProperty>(prop_base);
		auto element = prop->element_property();

		auto render_scalar = [&](ImGuiDataType type) -> bool {
			renderer->render_name(prop);
			bool is_changed = render_scalar_property(prop->address(context), element, type, prop->length(),
													 read_only || element->is_read_only());
			if (is_changed)
				prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));

			return is_changed;
		};

		if (element->is_a<Refl::BooleanProperty>())
		{
			renderer->render_name(prop);
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
			return render_scalar(find_imgui_data_type(integer));
		}
		else if (auto floating = prop_cast<Refl::FloatProperty>(element))
		{
			return render_scalar(find_imgui_data_type(floating));
		}
		else
		{
		}
		return false;
	}

	static bool render_matrix_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop         = prop_cast_checked<Refl::MatrixProperty>(prop_base);
		auto row_prop     = prop->row_property();
		const size_t rows = prop->rows();

		read_only = read_only | row_prop->is_read_only();

		bool is_changed = false;


		if (renderer->collapsing_header(prop))
		{
			ImGui::Indent();
			const char* names[] = {"rX", "rY", "rZ", "rW"};

			for (size_t i = 0; i < rows; i++)
			{
				renderer->create_row();

				renderer->next_prop_name(names[i]);

				void* address = prop->row_address(context, i);
				bool changed  = renderer->render_property(address, row_prop, read_only);

				if (changed)
				{
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));
					is_changed = changed;
				}
			}
			ImGui::Unindent();
		}

		return is_changed;
	}

	static bool render_enum_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop      = prop_cast_checked<Refl::EnumProperty>(prop_base);
		auto enum_inst = prop->enum_instance();

		renderer->render_name(prop);
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

	static bool render_string_property(PropertyRenderer* renderer, void* context, Refl::Property* prop, bool read_only)
	{
		renderer->render_name(prop);

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

	static bool render_name_property(PropertyRenderer* renderer, void* context, Refl::Property* prop, bool read_only)
	{
		renderer->render_name(prop);

		if (Name* value = prop->address_as<Name>(context))
		{
			ImGui::Text("%s", value->c_str());
		}

		return false;
	}

	static bool render_path_property(PropertyRenderer* renderer, void* context, Refl::Property* prop, bool read_only)
	{
		renderer->render_name(prop);
		Path* value = prop->address_as<Path>(context);

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

		return false;
	}

	static bool render_struct_property_internal(PropertyRenderer* renderer, void* context, void* struct_address,
												Refl::Property* prop, Refl::Struct* struct_instance, bool read_only)
	{
		bool is_changed = false;


		if (renderer->collapsing_header(prop))
		{
			push_props_id(struct_address, prop);
			ImGui::Indent();
			is_changed = renderer->render_struct_properties(struct_address, struct_instance, read_only);
			ImGui::Unindent();
			pop_props_id();

			if (is_changed)
			{
				Refl::PropertyChangedEvent event(context, Refl::PropertyChangeType::member_change, prop);
				prop->on_property_changed(event);
			}
		}
		return is_changed;
	}

	static bool render_object_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
	{
		Refl::ObjectProperty* prop = prop_cast_checked<Refl::ObjectProperty>(prop_base);
		auto object                = prop->object(context);

		if (object == nullptr)
		{
			renderer->mark_property_skipped();
			return false;
		}

		if (prop->is_composite())
		{
			return render_struct_property_internal(renderer, context, prop->object(context), prop, object->class_instance(),
												   read_only || prop->is_read_only());
		}
		else
		{
			const float size = ImGui::GetFrameHeight();
			auto object      = prop->object(context);

			bool changed = false;

			renderer->render_name(prop);
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

					if (new_object->class_instance()->is_a(prop->class_instance()))
					{
						prop->object(context, new_object);
						prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
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

	static bool render_struct_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
	{
		Refl::StructProperty* prop = prop_cast_checked<Refl::StructProperty>(prop_base);
		return render_struct_property_internal(renderer, context, prop->address(context), prop, prop->struct_instance(),
											   read_only || prop->is_read_only());
	}

	static bool render_array_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
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

		if (renderer->collapsing_header(prop))
		{
			ImGui::Indent();
			Refl::Property* element_prop = prop->element_property();

			size_t count = prop->length(context);

			static Vector<String> index_names;

			for (size_t i = 0; i < count; ++i)
			{
				renderer->create_row();
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

				if (i >= index_names.size())
					index_names.push_back(Strings::format("{}", i + 1));

				renderer->next_prop_name(index_names[i]);

				if (renderer->render_property(array_object, element_prop, element_prop->is_read_only()))
				{
					is_changed = true;
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));
				}

				ImGui::PopID();
			}

			ImGui::Unindent();
		}

		return is_changed;
	}

	static Refl::Object* refl_object_property_selector(Refl::Object* node, Refl::ClassInfo* info)
	{
		Refl::Object* result = nullptr;
		if (node->refl_class_info()->is_a(info))
		{
			if (ImGui::Selectable(node->display_name().c_str()))
			{
				result = node;
			}
		}

		if (auto scope = Refl::Object::instance_cast<Refl::ScopedType>(node))
		{
			for (auto& [name, child] : scope->childs())
			{
				auto child_result = refl_object_property_selector(child, info);

				if (!result)
					result = child_result;
			}
		}
		return result;
	}

	static bool render_refl_object_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop = prop_cast_checked<Refl::ReflObjectProperty>(prop_base);
		renderer->render_name(prop);

		auto current = prop->object(context);
		bool changed = false;

		if (ImGui::BeginCombo("##Combo", current ? current->display_name().c_str() : "None"))
		{
			bool use_none = ImGui::Selectable("None");

			if (use_none || (current = refl_object_property_selector(Refl::Object::static_root(), prop->info())))
			{
				prop->object(context, use_none ? nullptr : current);
				changed = true;
				prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
			}
			ImGui::EndCombo();
		}
		return changed;
	}

	static Refl::Class* refl_sub_class_property_selector(Refl::Class* node)
	{
		Refl::Class* result = nullptr;

		if (ImGui::Selectable(node->display_name().c_str()))
		{
			result = node;
		}

		for (auto& derived : node->derived_structs())
		{
			if (auto derived_class = Refl::Object::instance_cast<Refl::Class>(derived))
			{
				auto child_result = refl_sub_class_property_selector(derived_class);

				if (!result)
					result = child_result;
			}
		}

		return result;
	}

	static bool render_sub_class_property(PropertyRenderer* renderer, void* context, Refl::Property* prop_base, bool read_only)
	{
		auto prop = prop_cast_checked<Refl::SubClassProperty>(prop_base);
		renderer->render_name(prop);

		auto current = prop->class_instance(context);
		bool changed = false;

		if (ImGui::BeginCombo("##Combo", current ? current->display_name().c_str() : "None"))
		{
			bool use_none = ImGui::Selectable("None");

			if (use_none || (current = refl_sub_class_property_selector(prop->base_class())))
			{
				prop->class_instance(context, use_none ? nullptr : current);
				changed = true;
				prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
			}
			ImGui::EndCombo();
		}
		return changed;
	}

	static void on_preinit()
	{
		using T = PropertyRenderer;

		T::register_prop_renderer<Refl::BooleanProperty>(render_boolean_property);
		T::register_prop_renderer<Refl::IntegerProperty>(render_integer_property);
		T::register_prop_renderer<Refl::FloatProperty>(render_float_property);
		T::register_prop_renderer<Refl::VectorProperty>(render_vector_property);
		T::register_prop_renderer<Refl::MatrixProperty>(render_matrix_property);
		T::register_prop_renderer<Refl::EnumProperty>(render_enum_property);
		T::register_prop_renderer<Refl::StringProperty>(render_string_property);
		T::register_prop_renderer<Refl::NameProperty>(render_name_property);
		T::register_prop_renderer<Refl::PathProperty>(render_path_property);
		T::register_prop_renderer<Refl::StructProperty>(render_struct_property);
		T::register_prop_renderer<Refl::ObjectProperty>(render_object_property);
		T::register_prop_renderer<Refl::ArrayProperty>(render_array_property);
		T::register_prop_renderer<Refl::ReflObjectProperty>(render_refl_object_property);
		T::register_prop_renderer<Refl::SubClassProperty>(render_sub_class_property);
	}

	static bool script_render_property(PropertyRenderer* renderer, void* object, int type_id, Refl::Property* prop,
									   bool read_only, bool allow_script_call)
	{
		if (ScriptEngine::is_handle_type(type_id))
		{
			object = *reinterpret_cast<void**>(object);
		}

		return renderer->render_property(object, prop, read_only, allow_script_call);
	}

	static void on_init()
	{
		using T = PropertyRenderer;

		ScriptClassRegistrar::RefInfo info;
		info.no_count        = true;
		info.implicit_handle = true;

		auto r = ScriptClassRegistrar::reference_class("Engine::PropertyRenderer", info);
		ReflectionInitializeController().require("Engine::Refl::Property");

		r.method("void next_prop_name(const string&)", method_of<void>(&T::next_prop_name));
		r.method("void render_name(Refl::Property@ prop)", &T::render_name);

		r.method("bool render_property(?& object, Refl::Property@ prop, bool read_only = false, bool allow_script_call = true)",
				 script_render_property);
	}

	static PreInitializeController pre_init(on_preinit);
	static ReflectionInitializeController init(on_init, "Engine::PropertyRenderer");
}// namespace Engine
