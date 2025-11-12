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
#include <UI/primitives.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/property_renderer.hpp>
#include <imfilebrowser.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	struct PropertyChangedEvent : Refl::PropertyChangedEvent {
		PropertyChangedEvent(void* ctx, Refl::Property* prop, Refl::PropertyChangedEvent* owner)
		    : Refl::PropertyChangedEvent(ctx, prop, owner)
		{
			if (owner)
			{
				owner->member_event = nullptr;
			}
		}

		~PropertyChangedEvent()
		{
			if (owner_event)
			{
				owner_event->member_event = nullptr;
			}
		}
	};

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

	static FORCE_INLINE bool has_only_one_property(Refl::Struct* self)
	{
		uint_t count = 0;

		while (self && count < 2)
		{
			count += static_cast<uint_t>(self->properties().size());
			self = self->parent();
		}

		return count < 2;
	}

	struct ScopedPropID {
		ScopedPropID(const void* object, Refl::Property* prop) { push_props_id(object, prop); }
		~ScopedPropID() { pop_props_id(); }
	};

	using RendererFunc = PropertyRenderer::RendererFunc;

	const RendererFunc& PropertyRenderer::Context::renderer(const Refl::ClassInfo* refl_class) const
	{
		if (auto ptr = renderer_ptr(refl_class))
			return *ptr;
		return default_value_of<RendererFunc>();
	}

	const RendererFunc* PropertyRenderer::Context::renderer_ptr(const Refl::ClassInfo* refl_class) const
	{
		for (const Context* ctx = this; ctx; ctx = ctx->prev())
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
			auto ctx = renderer_context();

			bool status  = false;
			bool founded = false;

			for (auto current = ctx; current && !founded; current = current->prev())
			{
				if ((founded = static_cast<bool>(current->on_begin_rendering)))
				{
					if ((status = current->on_begin_rendering(this)))
					{
						m_property_index = 0;
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
					current->on_end_rendering(this, status);
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

	PropertyRenderer::PropertiesMap& PropertyRenderer::build_properties_map(PropertiesMap& map, Refl::Struct* self)
	{
		for (; self; self = self->parent())
		{
			for (Refl::Property* prop : self->properties())
			{
				map[prop->group()].push_back(prop);
			}
		}

		return map;
	}

	PropertyRenderer::PropertiesMap& PropertyRenderer::build_properties_map(Refl::Struct* self)
	{
		auto& map = m_properties[self];
		map.clear();

		return build_properties_map(map, self);
	}

	PropertyRenderer& PropertyRenderer::object(Object* object, bool reset)
	{
		m_object = object;

		if (reset)
		{
			m_properties.clear();
			userdata.clear();
			build_properties_map(struct_instance());
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
			return build_properties_map(self);
		}

		return map;
	}

	PropertyRenderer& PropertyRenderer::propagate_property_event()
	{
		Refl::PropertyChangedEvent* event = m_event;

		while (event)
		{
			event->property->on_property_changed(*event);
			event = event->owner_event;
		}

		return *this;
	}

	PropertyRenderer& PropertyRenderer::propagate_property_event(void* ctx, Refl::Property* property)
	{
		PropertyChangedEvent event(ctx, property, m_event);
		m_event = &event;
		propagate_property_event();
		m_event = event.owner_event;
		return *this;
	}

	PropertyRenderer& PropertyRenderer::next_name(const String& name)
	{
		m_next_name = name;
		return *this;
	}

	const String& PropertyRenderer::property_name(const String& name)
	{
		const String& overrided = m_property_names_stack.back();

		if (overrided.empty())
		{
			return name;
		}

		return overrided;
	}

	const String& PropertyRenderer::property_name(Refl::Property* prop, const void* context)
	{
		const String& overrided = m_property_names_stack.back();

		if (overrided.empty())
		{
			return prop->property_name(context);
		}

		return overrided;
	}

	void* PropertyRenderer::property_context() const
	{
		return m_event->context;
	}

	Refl::Property* PropertyRenderer::property() const
	{
		return m_event->property;
	}

	bool PropertyRenderer::render_property(void* object, Refl::Property* prop, bool read_only)
	{
		if (prop->is_hidden())
			return false;

		Context* ctx = renderer_context();

		read_only = read_only || prop->is_read_only();
		ScopedPropID prop_id(object, prop);

		if (auto renderer = ctx->renderer_ptr(prop->refl_class_info()))
		{
			PropertyChangedEvent event(object, prop, m_event);
			m_event = &event;
			m_property_names_stack.emplace_back(std::move(m_next_name));

			bool result = (*renderer)(this, prop, read_only);

			m_property_names_stack.pop_back();
			m_event = event.owner_event;
			return result;
		}

		return false;
	}

	bool PropertyRenderer::render_struct_properties(void* object, Refl::Struct* struct_class, bool read_only)
	{
		const Context* ctx     = renderer_context();
		bool has_changed_props = false;

		for (auto& [group, props] : properties_map(struct_class))
		{
			bool open = true;

			if (!group.empty())
			{
				for (const Context* current = ctx; current; current = current->prev())
				{
					if (current->on_begin_group)
					{
						open = current->on_begin_group(this, group);
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
						current->on_end_group(this, group, open);
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
		ImGui::TableSetColumnIndex(1);
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
			renderer->propagate_property_event();
			return true;
		}

		return false;
	}

	static bool render_scalar_property(void* context, Refl::Property* prop, PropertyRenderer* renderer, ImGuiDataType type,
	                                   int_t components, bool read_only)
	{
		ImGuiInputTextFlags flags = (read_only ? ImGuiInputTextFlags_ReadOnly : 0);
		void* address             = prop->address(context);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::InputScalarN("##value", type, address, components, nullptr, nullptr, nullptr, flags))
		{
			renderer->propagate_property_event(context, prop);
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

	static bool render_integer_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		ImGui::TableNextRow();
		auto prop = prop_cast_checked<Refl::IntegerProperty>(prop_base);
		render_name(renderer, prop_base);
		return render_scalar_property(renderer->property_context(), prop, renderer, find_imgui_data_type(prop), 1, read_only);
	}

	static bool render_float_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);
		auto float_prop = prop_cast_checked<Refl::FloatProperty>(prop);
		return render_scalar_property(renderer->property_context(), prop, renderer, find_imgui_data_type(float_prop), 1,
		                              read_only);
	}

	static bool render_vector_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop    = prop_cast_checked<Refl::VectorProperty>(prop_base);
		auto element = prop->element_property();

		void* context = renderer->property_context();

		auto render_scalar = [&](ImGuiDataType type) -> bool {
			ImGui::TableNextRow();
			render_name(renderer, prop);
			void* address = prop->address(context);
			return render_scalar_property(address, element, renderer, type, prop->length(), read_only || element->is_read_only());
		};

		if (element->is_a<Refl::BooleanProperty>())
		{
			ImGui::TableNextRow();
			render_name(renderer, prop);
			bool* context = prop->address_as<bool>(renderer->property_context());

			char name[] = "##value0";
			for (size_t i = 0, count = prop->length(); i < count; ++i)
			{
				bool& value = *(context + i);
				name[7]     = '0' + i;

				if (ImGui::Checkbox(name, &value) && !read_only)
				{
					renderer->propagate_property_event(&value, element);
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
				renderer->next_name(names[i]);

				void* context = renderer->property_context();
				void* address = prop->row_address(context, i);
				is_changed    = renderer->render_property(address, row_prop, read_only) || is_changed;
			}
			ImGui::Unindent();
		}

		return is_changed;
	}

	static bool render_quaternion_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop              = prop_cast_checked<Refl::QuaternionProperty>(prop_base);
		Quaternion* quaternion = prop->address_as<Quaternion>(renderer->property_context());
		Vector3f degrees       = Math::degrees(Math::euler_angles(*quaternion));

		ImGui::TableNextRow();
		render_name(renderer, prop);

		ImGuiInputTextFlags flags = (read_only ? ImGuiInputTextFlags_ReadOnly : 0);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::InputFloat3("##value", &degrees.x, "%.3f", flags))
		{
			*quaternion = Quaternion(Math::radians(degrees));
			renderer->propagate_property_event();
			return true;
		}

		return false;
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
					renderer->propagate_property_event();
					is_changed = true;
				}
			}

			ImGui::EndCombo();
		}

		return is_changed;
	}

	static bool render_color_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);

		void* context = renderer->property_context();

		Color* color       = prop->address_as<Color>(context);
		LinearColor linear = {
		        static_cast<float>(color->r) / 255.f,
		        static_cast<float>(color->g) / 255.f,
		        static_cast<float>(color->b) / 255.f,
		        static_cast<float>(color->a) / 255.f,
		};

		uint32_t flags = ImGuiColorEditFlags_Uint8;

		if (read_only)
			flags |= ImGuiColorEditFlags_NoInputs;

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::ColorEdit4("###value", &linear.x, flags))
		{
			(*color) = linear;
			renderer->propagate_property_event();
			return true;
		}

		return false;
	}

	static bool render_linear_color_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);

		void* context      = renderer->property_context();
		LinearColor* color = prop->address_as<LinearColor>(context);

		uint32_t flags = ImGuiColorEditFlags_Float;

		if (read_only)
			flags |= ImGuiColorEditFlags_NoInputs;

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::ColorEdit4("###value", &color->x, flags))
		{
			renderer->propagate_property_event();
			return true;
		}

		return false;
	}

	static bool render_string_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		ImGui::TableNextRow();
		render_name(renderer, prop);

		void* context = renderer->property_context();
		if (String* value = prop->address_as<String>(context))
		{
			auto flags = ImGuiInputTextFlags_EnterReturnsTrue | (read_only ? ImGuiInputTextFlags_ReadOnly : 0);

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::InputText("##Value", *value, flags))
			{
				renderer->propagate_property_event();
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
			renderer->propagate_property_event();
		}

		if (ImGui::TableGetColumnCount() > 2)
		{
			ImGui::TableSetColumnIndex(2);

			if (UI::icon_button(UI::select_icon, "##Select", size))
			{
				Function<void(const Path&)> callback = [renderer, value, &str](const Path& path) {
					*value = path;
					renderer->propagate_property_event();
					str = value->str();
				};

				imgui_window->widgets.create<ImGuiOpenFile>()->on_select.push(callback);
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

		bool is_changed   = false;
		bool is_collapsed = false;

		if (prop->is_inline())
		{
			is_collapsed = true;
			renderer->next_name(renderer->property_name());
		}
		else if (prop->is_inline_single_field())
		{
			if (has_only_one_property(struct_instance))
			{
				is_collapsed = true;
				renderer->next_name(renderer->property_name());
			}
			else
			{
				ImGui::TableNextRow();
				is_collapsed = collapsing_header(renderer, prop);
			}
		}
		else
		{
			ImGui::TableNextRow();
			is_collapsed = collapsing_header(renderer, prop);
		}

		if (is_collapsed)
		{
			push_props_id(struct_address, prop);
			ImGui::Indent();
			is_changed = renderer->render_struct_properties(struct_address, struct_instance, read_only);
			ImGui::Unindent();
			pop_props_id();
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
			ImGui::Image(Icons::find_icon(object), {5.f * size, 5.f * size});

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
						renderer->propagate_property_event();
						changed = true;
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopID();

			if (!read_only && ImGui::TableGetColumnCount() > 2)
			{
				ImGui::TableSetColumnIndex(2);

				if (UI::icon_button(UI::rotate_icon, "###reset", size))
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

			if (!read_only && UI::icon_button(UI::plus_icon, "##emplace_back", size))
			{
				prop->emplace_back(context);
				renderer->propagate_property_event();
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
				renderer->next_name(prop->index_name(context, i));

				if (renderer->render_property(array_object, element_prop, element_prop->is_read_only()))
				{
					is_changed = true;
				}

				if (ImGui::TableGetColumnCount() > 2)
				{
					ImGui::TableSetColumnIndex(2);

					if (!read_only && UI::icon_button(UI::minus_icon, "##erase", size))
					{
						prop->erase(context, i);
						--count;
						ImGui::PopID();
						continue;
					}
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
				renderer->propagate_property_event();
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
				renderer->propagate_property_event();
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
						renderer->propagate_property_event();
					}
					ImGui::PopID();
				}
			}
			ImGui::Unindent();
		}

		return changed;
	}

	static bool render_virtual_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		Refl::VirtualProperty* prop = prop_cast_checked<Refl::VirtualProperty>(prop_base);
		void* context               = renderer->property_context();

		Any value = prop->getter(context);
		renderer->next_name(renderer->property_name(prop, context));

		const bool changed = renderer->render_property(value.address(), prop->property(), read_only);

		if (changed)
		{
			prop->setter(context, value);
			renderer->propagate_property_event();
		}

		return changed;
	}

	static void on_preinit()
	{
		auto ctx = PropertyRenderer::static_global_renderer_context();

		ctx->on_begin_rendering = [](PropertyRenderer*) -> bool {
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

		ctx->on_end_rendering = [](PropertyRenderer*, bool status) { ImGui::EndTable(); };

		ctx->on_begin_group = [](PropertyRenderer*, const String& group) -> bool {
			ImGui::TableNextRow();
			bool result = collapsing_header(group.c_str(), group.c_str());
			ImGui::Indent();
			return result;
		};

		ctx->on_end_group = [](PropertyRenderer*, const String& group, bool status) { ImGui::Unindent(); };

		ctx->renderer<Refl::BooleanProperty>(render_boolean_property);
		ctx->renderer<Refl::IntegerProperty>(render_integer_property);
		ctx->renderer<Refl::FloatProperty>(render_float_property);
		ctx->renderer<Refl::VectorProperty>(render_vector_property);
		ctx->renderer<Refl::MatrixProperty>(render_matrix_property);
		ctx->renderer<Refl::QuaternionProperty>(render_quaternion_property);
		ctx->renderer<Refl::EnumProperty>(render_enum_property);
		ctx->renderer<Refl::ColorProperty>(render_color_property);
		ctx->renderer<Refl::LinearColorProperty>(render_linear_color_property);
		ctx->renderer<Refl::StringProperty>(render_string_property);
		ctx->renderer<Refl::NameProperty>(render_name_property);
		ctx->renderer<Refl::PathProperty>(render_path_property);
		ctx->renderer<Refl::StructProperty>(render_struct_property);
		ctx->renderer<Refl::ObjectProperty>(render_object_property);
		ctx->renderer<Refl::ArrayProperty>(render_array_property);
		ctx->renderer<Refl::ReflObjectProperty>(render_refl_object_property);
		ctx->renderer<Refl::SubClassProperty>(render_sub_class_property);
		ctx->renderer<Refl::FlagsProperty>(render_flags_property);
		ctx->renderer<Refl::VirtualProperty>(render_virtual_property);
	}

	static PreInitializeController pre_init(on_preinit);
}// namespace Engine
