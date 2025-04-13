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
#include <Widgets/imgui_windows.hpp>
#include <Widgets/property_renderer.hpp>
#include <imfilebrowser.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
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
		ScopedPropID(const void* object, Refl::Property* prop) { push_props_id(object, prop); }
		~ScopedPropID() { pop_props_id(); }
	};

	const PropertyRenderer::RendererFunc& PropertyRenderer::Context::renderer(const Refl::ClassInfo* refl_class)
	{
		if (auto ptr = renderer_ptr(refl_class))
			return *ptr;
		return default_value_of<RendererFunc>();
	}

	const PropertyRenderer::RendererFunc* PropertyRenderer::Context::renderer_ptr(const Refl::ClassInfo* refl_class)
	{
		for (Context* ctx = this; ctx; ctx = ctx->prev())
		{
			auto it = ctx->m_renderers.find(refl_class);
			if (it != ctx->m_renderers.end())
				return &it->second;
		}
		return nullptr;
	}

	PropertyRenderer::Context& PropertyRenderer::Context::renderer(const Refl::ClassInfo* refl_class, const RendererFunc& func)
	{
		m_renderers[refl_class] = func;
		return *this;
	}

	PropertyRenderer::PropertyRenderer() : m_object(nullptr)
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
		render();
		ImGui::End();

		return open;
	}

	PropertyRenderer& PropertyRenderer::render()
	{
		if (m_object)
		{
			auto ctx = m_ctx ? m_ctx : static_global_renderer_context();

			bool status  = false;
			bool founded = false;

			for (auto current = ctx; current && !founded; current = current->prev())
			{
				if ((founded = static_cast<bool>(current->on_begin_rendering)))
				{
					if ((status = current->on_begin_rendering()))
					{
						render_struct_properties(m_object, m_object->class_instance(), false);
					}
				}
			}

			if (!founded)
				return *this;

			for (auto current = ctx; current; current = current->prev())
			{
				if (current->on_end_rendering)
				{
					current->on_end_rendering(status);
					return *this;
				}
			}
		}
		return *this;
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

	PropertyRenderer& PropertyRenderer::object(Object* object, bool reset)
	{
		m_object = object;

		if (reset)
		{
			m_properties.clear();
			userdata.clear();
			build_props_map(struct_instance());
		}
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

	PropertyRenderer& PropertyRenderer::push_name(const String& name, uint16_t usages)
	{
		m_prop_names_stack.emplace_back(name, usages);
		return *this;
	}

	PropertyRenderer& PropertyRenderer::pop_name()
	{
		trinex_check(m_prop_names_stack.size() > 0, "Names stack is empty. push_name/pop_name count mismatch!");
		m_prop_names_stack.pop_back();
		return *this;
	}

	const String& PropertyRenderer::property_name(const String& name)
	{
		if (!m_prop_names_stack.empty())
		{
			auto& back = m_prop_names_stack.back();
			if (back.usages > 0)
			{
				--back.usages;
				return back.name;
			}
		}
		return name;
	}

	const String& PropertyRenderer::property_name(Refl::Property* prop, const void* context)
	{
		if (!m_prop_names_stack.empty())
		{
			auto& back = m_prop_names_stack.back();
			if (back.usages > 0)
			{
				--back.usages;
				return back.name;
			}
		}
		return prop->property_name(context);
	}

	void* PropertyRenderer::property_context(size_t stack_offset) const
	{
		trinex_check(stack_offset < m_context_stack.size(), "Stack offset is out of range!");
		return *(m_context_stack.end() - stack_offset);
	}

	bool PropertyRenderer::render_property(void* object, Refl::Property* prop, bool read_only)
	{
		if (prop->is_hidden())
			return false;

		Context* ctx = m_ctx ? m_ctx : static_global_renderer_context();

		read_only = read_only || prop->is_read_only();
		ScopedPropID prop_id(object, prop);

		if (auto renderer = ctx->renderer_ptr(prop->refl_class_info()))
		{
			m_context_stack.push_back(object);
			bool result = (*renderer)(this, prop, read_only);
			m_context_stack.pop_back();
			return result;
		}

		return false;
	}

	bool PropertyRenderer::render_struct_properties(void* object, Refl::Struct* struct_class, bool read_only)
	{
		bool has_changed_props = false;

		const Context* ctx = m_ctx ? m_ctx : static_global_renderer_context();

		for (auto& [group, props] : properties_map(struct_class))
		{
			bool open = true;

			if (!group.empty())
			{
				for (const Context* current = ctx; current; current = current->prev())
				{
					if (current->on_begin_group)
					{
						open = current->on_begin_group(group);
						break;
					}
				}
			}

			if (open)
			{
				for (auto& prop : props)
				{
					if (render_property(object, prop, read_only))
						has_changed_props = true;
				}
			}

			if (!group.empty())
			{
				for (const Context* current = ctx; current; current = current->prev())
				{
					if (current->on_end_group)
					{
						current->on_end_group(group, open);
						break;
					}
				}
			}
		}

		return has_changed_props;
	}


	const char* PropertyRenderer::name() const
	{
		return static_name();
	}

	const char* PropertyRenderer::static_name()
	{
		return "editor/Properties"_localized;
	}

	PropertyRenderer::Context* PropertyRenderer::static_global_renderer_context()
	{
		static Context ctx;
		return &ctx;
	}

	//////////////////////// PROPERTY RENDERERS ////////////////////////

	static void render_name(PropertyRenderer* renderer, Refl::Property* prop)
	{
		ImGui::TableSetColumnIndex(0);
		const String& name = renderer->property_name(prop, renderer->property_context());
		ImGui::Text("%s", name.c_str());

		ImGui::TableSetColumnIndex(1);
		auto& tooltip = prop->tooltip();

		if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("%s", tooltip.c_str());
		}
	}

	static void render_name(PropertyRenderer* renderer, const String& name)
	{
		ImGui::TableSetColumnIndex(0);
		const String& result_name = renderer->property_name(name);
		ImGui::Text("%s", result_name.c_str());
	}

	static bool collapsing_header(const void* id, const char* name)
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

		bool result = ImGui::TreeNodeEx(id, flags, "%s", name);

		window->ClipRect       = clip_rect;
		window->ParentWorkRect = parent_work_rect;
		return result;
	}

	static bool collapsing_header(PropertyRenderer* renderer, Refl::Property* prop)
	{
		const String& name = renderer->property_name(prop, renderer->property_context());
		return collapsing_header(prop, name.c_str());
	}

	static bool render_boolean_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);
		void* context       = renderer->property_context();
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

	static bool render_scalar_property_colored(void* context, Refl::Property* prop, ImGuiDataType type, int_t components,
											   bool read_only, bool is_color = false)
	{
		ImGuiInputTextFlags flags = (read_only ? ImGuiInputTextFlags_ReadOnly : 0);
		void* address             = prop->address(context);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		bool changed = false;

		if (is_color && (components == 3 || components == 4) && type == ImGuiDataType_Float)
		{
			if (components == 3)
			{
				changed = ImGui::ColorEdit3("##value", reinterpret_cast<float*>(address), ImGuiColorEditFlags_Float);
			}
			else
			{
				changed = ImGui::ColorEdit4("##value", reinterpret_cast<float*>(address), ImGuiColorEditFlags_Float);
			}
		}
		else
		{
			changed = ImGui::InputScalarN("##value", type, address, components, nullptr, nullptr, nullptr, flags);
		}

		if (changed)
		{
			prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
		}
		return changed;
	}

	static bool render_scalar_property(void* context, Refl::Property* prop, ImGuiDataType type, int_t components, bool read_only)
	{
		return render_scalar_property_colored(context, prop, type, components, read_only, false);
	}

	static ImGuiDataType find_imgui_data_type(Refl::IntegerProperty* prop)
	{
		return (std::countr_zero(prop->size()) * 2) + static_cast<ImGuiDataType>(prop->is_unsigned());
	}

	static ImGuiDataType find_imgui_data_type(Refl::FloatProperty* prop)
	{
		return prop->size() == 4 ? ImGuiDataType_Float : ImGuiDataType_Double;
	}

	static bool render_integer_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		ImGui::TableNextRow();
		auto prop = prop_cast_checked<Refl::IntegerProperty>(prop_base);
		render_name(renderer, prop_base);
		return render_scalar_property(renderer->property_context(), prop, find_imgui_data_type(prop), 1, read_only);
	}

	static bool render_float_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);
		auto float_prop = prop_cast_checked<Refl::FloatProperty>(prop);
		return render_scalar_property(renderer->property_context(), prop, find_imgui_data_type(float_prop), 1, read_only);
	}

	static bool render_vector_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop    = prop_cast_checked<Refl::VectorProperty>(prop_base);
		auto element = prop->element_property();

		void* context = renderer->property_context();

		auto render_scalar = [&](ImGuiDataType type) -> bool {
			ImGui::TableNextRow();
			render_name(renderer, prop);
			bool is_changed = render_scalar_property_colored(prop->address(context), element, type, prop->length(),
															 read_only || element->is_read_only(), prop->is_color());
			if (is_changed)
				prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));

			return is_changed;
		};

		if (element->is_a<Refl::BooleanProperty>())
		{
			ImGui::TableNextRow();
			render_name(renderer, prop);
			void* context       = renderer->property_context();
			bool* value_address = prop->address_as<bool>(renderer->property_context());

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

		return false;
	}

	static bool render_matrix_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop         = prop_cast_checked<Refl::MatrixProperty>(prop_base);
		auto row_prop     = prop->row_property();
		const size_t rows = prop->rows();

		read_only       = read_only | row_prop->is_read_only();
		bool is_changed = false;

		ImGui::TableNextRow();
		if (collapsing_header(renderer, prop))
		{
			ImGui::Indent();
			const char* names[] = {"rX", "rY", "rZ", "rW"};

			for (size_t i = 0; i < rows; i++)
			{
				renderer->push_name(names[i]);

				void* context = renderer->property_context();
				void* address = prop->row_address(context, i);
				bool changed  = renderer->render_property(address, row_prop, read_only);

				if (changed)
				{
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));
					is_changed = changed;
				}

				renderer->pop_name();
			}
			ImGui::Unindent();
		}

		return is_changed;
	}

	static bool render_enum_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop      = prop_cast_checked<Refl::EnumProperty>(prop_base);
		auto enum_inst = prop->enum_instance();
		void* context  = renderer->property_context();

		ImGui::TableNextRow();
		render_name(renderer, prop);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

		EnumerateType value       = prop->value(context);
		const auto* current_entry = enum_inst->entry(value);

		bool is_changed = false;

		if (current_entry && ImGui::BeginCombo("##ComboValue", current_entry->name.c_str()))
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

	static bool render_string_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);

		void* context = renderer->property_context();
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

	static bool render_name_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);

		void* context = renderer->property_context();
		if (Name* value = prop->address_as<Name>(context))
		{
			ImGui::Text("%s", value->c_str());
		}

		return false;
	}

	static bool render_path_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);

		void* context = renderer->property_context();
		Path* value   = prop->address_as<Path>(context);

		ImGuiWindow* imgui_window = ImGuiWindow::current();
		const float size          = ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y;

		Any& data = renderer->userdata.get(context);

		if (!data.has_value())
		{
			data = value->str();
		}

		String& str = data.cast<String&>();

		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::InputText("##Path", str, read_only ? ImGuiInputTextFlags_ReadOnly : 0))
		{
			*value = str;
			prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
		}

		if (ImGui::TableGetColumnCount() > 2)
		{
			ImGui::TableSetColumnIndex(2);
			if (ImGui::ImageButton(Icons::icon(Icons::Select), {size, size}))
			{
				Function<void(const Path&)> callback = [context, prop, value, &str](const Path& path) {
					*value = path;
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
					str = value->str();
				};

				imgui_window->widgets_list.create<ImGuiOpenFile>()->on_select.push(callback);
			}
		}

		return false;
	}

	static bool render_struct_property_internal(PropertyRenderer* renderer, void* struct_address, Refl::Property* prop,
												Refl::Struct* struct_instance, bool read_only)
	{
		auto& properties_map = renderer->properties_map(struct_instance);

		if (properties_map.empty())
			return false;

		bool is_changed = false;

		const bool is_inlined = prop->inline_single_field_structs() &&//
								properties_map.size() == 1 &&         //
								properties_map.begin()->second.size() == 1;

		if (is_inlined)
		{
			is_changed = renderer->render_property(struct_address, properties_map.begin()->second.front(), read_only);
		}
		else
		{
			ImGui::TableNextRow();

			if (collapsing_header(renderer, prop))
			{
				push_props_id(struct_address, prop);
				ImGui::Indent();
				is_changed = renderer->render_struct_properties(struct_address, struct_instance, read_only);
				ImGui::Unindent();
				pop_props_id();
			}
		}

		if (is_changed)
		{
			void* context = renderer->property_context();
			Refl::PropertyChangedEvent event(context, Refl::PropertyChangeType::member_change, prop);
			prop->on_property_changed(event);
		}
		return is_changed;
	}

	static bool render_object_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		Refl::ObjectProperty* prop = prop_cast_checked<Refl::ObjectProperty>(prop_base);
		void* context              = renderer->property_context();
		auto object                = prop->object(context);

		if (prop->is_composite())
		{
			if (object == nullptr)
			{
				return false;
			}

			return render_struct_property_internal(renderer, object, prop, object->class_instance(),
												   read_only || prop->is_read_only());
		}
		else
		{
			const float size = ImGui::GetFrameHeight();
			auto object      = prop->object(context);

			bool changed = false;

			ImGui::TableNextRow();
			render_name(renderer, prop);
			ImGui::TableSetColumnIndex(1);

			ImGui::PushID("##Image");
			ImGui::Image(Icons::find_icon(object), {100, 100});

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

			if (!read_only && ImGui::TableGetColumnCount() > 2)
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

	static bool render_struct_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		Refl::StructProperty* prop = prop_cast_checked<Refl::StructProperty>(prop_base);
		return render_struct_property_internal(renderer, prop->address(renderer->property_context()), prop,
											   prop->struct_instance(), read_only || prop->is_read_only());
	}

	static bool render_array_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop        = prop_cast_checked<Refl::ArrayProperty>(prop_base);
		bool is_changed  = false;
		const float size = ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y;

		void* context = renderer->property_context();

		ImGui::TableNextRow();
		if (ImGui::TableGetColumnCount() > 2)
		{
			ImGui::TableSetColumnIndex(2);

			if (!read_only && ImGui::ImageButton(Icons::icon(Icons::Add), {size, size}))
			{
				prop->emplace_back(context);
				prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::array_add, prop));
				is_changed = true;
			}
		}

		if (collapsing_header(renderer, prop))
		{
			ImGui::Indent();
			Refl::Property* element_prop = prop->element_property();

			size_t count = prop->length(context);

			for (size_t i = 0; i < count; ++i)
			{
				ImGui::PushID(i);

				void* array_object = prop->at(context, i);
				renderer->push_name(prop->index_name(context, i));

				if (renderer->render_property(array_object, element_prop, element_prop->is_read_only()))
				{
					is_changed = true;
					prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::member_change, prop));
				}

				if (ImGui::TableGetColumnCount() > 2)
				{
					ImGui::TableSetColumnIndex(2);

					if (!read_only && ImGui::ImageButton(Icons::icon(Icons::Remove), {size, size}))
					{
						prop->erase(context, i);
						--count;
						ImGui::PopID();
						continue;
					}
				}

				renderer->pop_name();

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

	static bool render_refl_object_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		ImGui::TableNextRow();
		auto prop = prop_cast_checked<Refl::ReflObjectProperty>(prop_base);
		render_name(renderer, prop);

		void* context = renderer->property_context();

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

	static bool render_sub_class_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		ImGui::TableNextRow();
		auto prop = prop_cast_checked<Refl::SubClassProperty>(prop_base);
		render_name(renderer, prop);

		void* context = renderer->property_context();
		auto current  = prop->class_instance(context);
		bool changed  = false;

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

	static bool render_flags_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop = prop_cast_checked<Refl::FlagsProperty>(prop_base);

		bool changed = false;

		struct Entry {
			String name;
			EnumerateType value;
		};

		ImGui::TableNextRow();

		if (collapsing_header(renderer, prop))
		{
			void* context   = renderer->property_context();
			uint32_t* flags = prop->address_as<uint32_t>(context);
			auto& userdata  = renderer->userdata.get(flags);

			if (!userdata.has_value())
			{
				auto& enum_entries = prop->enum_instance()->entries();

				userdata               = Vector<Entry>();
				Vector<Entry>& entries = userdata.cast<Vector<Entry>&>();
				entries.reserve(enum_entries.size());

				for (auto& entry : enum_entries)
				{
					entries.emplace_back(Strings::make_sentence(entry.name.to_string()), entry.value);
				}
			}

			Vector<Entry>& entries = userdata.cast<Vector<Entry>&>();

			ImGui::Indent();
			{
				for (auto& entry : entries)
				{
					ImGui::TableNextRow();
					render_name(renderer, entry.name);
					ImGui::PushID(entry.value);
					if (ImGui::CheckboxFlags("##Flags", flags, entry.value))
					{
						changed = true;
						prop->on_property_changed(Refl::PropertyChangedEvent(context, Refl::PropertyChangeType::value_set, prop));
					}
					ImGui::PopID();
				}
			}
			ImGui::Unindent();
		}

		return changed;
	}

	static void on_preinit()
	{
		auto ctx = PropertyRenderer::static_global_renderer_context();

		ctx->on_begin_rendering = []() -> bool {
			bool status = ImGui::BeginTable("###properties", 3,
											ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner);
			if (status)
			{
				ImGui::TableSetupColumn("##Column1", ImGuiTableColumnFlags_WidthStretch, 0.45);
				ImGui::TableSetupColumn("##Column2", ImGuiTableColumnFlags_WidthStretch, 0.45);
				ImGui::TableSetupColumn("##Column3", ImGuiTableColumnFlags_WidthStretch, 0.1);
			}

			return status;
		};

		ctx->on_end_rendering = [](bool status) { ImGui::EndTable(); };

		ctx->on_begin_group = [](const String& group) -> bool {
			ImGui::TableNextRow();
			bool result = collapsing_header(group.c_str(), group.c_str());
			ImGui::Indent();
			return result;
		};

		ctx->on_end_group = [](const String& group, bool status) { ImGui::Unindent(); };

		ctx->renderer<Refl::BooleanProperty>(render_boolean_property);
		ctx->renderer<Refl::IntegerProperty>(render_integer_property);
		ctx->renderer<Refl::FloatProperty>(render_float_property);
		ctx->renderer<Refl::VectorProperty>(render_vector_property);
		ctx->renderer<Refl::MatrixProperty>(render_matrix_property);
		ctx->renderer<Refl::EnumProperty>(render_enum_property);
		ctx->renderer<Refl::StringProperty>(render_string_property);
		ctx->renderer<Refl::NameProperty>(render_name_property);
		ctx->renderer<Refl::PathProperty>(render_path_property);
		ctx->renderer<Refl::StructProperty>(render_struct_property);
		ctx->renderer<Refl::ObjectProperty>(render_object_property);
		ctx->renderer<Refl::ArrayProperty>(render_array_property);
		ctx->renderer<Refl::ReflObjectProperty>(render_refl_object_property);
		ctx->renderer<Refl::SubClassProperty>(render_sub_class_property);
		ctx->renderer<Refl::FlagsProperty>(render_flags_property);
	}

	static PreInitializeController pre_init(on_preinit);
}// namespace Engine
