#include <Core/constants.hpp>
#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/algorithm.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/filesystem/path.hpp>
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
#include <UI/property_renderer.hpp>
#include <imfilebrowser.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine::UI
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

	static FORCE_INLINE size_t properties_count(Refl::Struct* self)
	{
		size_t count = 0;

		while (self)
		{
			count += self->properties().size();
			self = self->parent();
		}

		return count;
	}

	static Refl::Property** collect_properties(Refl::Struct* self, size_t& count)
	{
		count                       = properties_count(self);
		Refl::Property** properties = StackAllocator<Refl::Property*>::allocate(count);
		Refl::Property** current    = properties;

		while (self)
		{
			auto& properties = self->properties();

			for (Refl::Property* prop : properties)
			{
				(*current++) = prop;
			}

			self = self->parent();
		}

		return properties;
	}

	struct ScopedPropID {
		ScopedPropID(const void* object, Refl::Property* prop) { push_props_id(object, prop); }
		~ScopedPropID() { pop_props_id(); }
	};

	using RendererFunc = PropertyRenderer::RendererFunc;

	PropertyRenderingFlags PropertyRenderer::Context::flags() const
	{
		if (m_prev)
			return m_prev->flags();
		return PropertyRenderingFlags::Undefined;
	}

	bool PropertyRenderer::Context::on_begin_rendering(PropertyRenderer* renderer)
	{
		if (m_prev)
			return m_prev->on_begin_rendering(renderer);

		return true;
	}

	PropertyRenderer::Context& PropertyRenderer::Context::on_end_rendering(PropertyRenderer* renderer, bool rendered)
	{
		if (m_prev)
			m_prev->on_end_rendering(renderer, rendered);

		return *this;
	}

	bool PropertyRenderer::Context::on_begin_group(PropertyRenderer* renderer, const String& group)
	{
		if (m_prev)
			return m_prev->on_begin_group(renderer, group);

		return true;
	}

	PropertyRenderer::Context& PropertyRenderer::Context::on_end_group(PropertyRenderer* renderer, const String& group, bool open)
	{
		if (m_prev)
			m_prev->on_end_group(renderer, group, open);
		return *this;
	}

	uint_t PropertyRenderer::Context::columns() const
	{
		if (m_prev)
			return m_prev->columns();
		return 0;
	}

	PropertyRenderer::Context& PropertyRenderer::Context::column(uint_t index)
	{
		if (m_prev)
			m_prev->column(index);
		return *this;
	}

	PropertyRenderer::Context& PropertyRenderer::Context::next_row(ImGuiTableRowFlags row_flags, float row_min_height)
	{
		if (m_prev)
			m_prev->next_row(row_flags, row_min_height);
		return *this;
	}

	float PropertyRenderer::Context::cell_width() const
	{
		if (m_prev)
			return m_prev->cell_width();
		return 0.f;
	}

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

	StringView PropertyRenderer::property_name(StringView name)
	{
		StringView overrided = m_names_stack.back();

		if (overrided.empty())
		{
			return name;
		}

		return overrided;
	}

	StringView PropertyRenderer::property_name(Refl::Property* prop, const void* context)
	{
		StringView overrided = m_names_stack.back();

		if (overrided.empty())
		{
			return prop->property_name(context);
		}

		return overrided;
	}

	bool PropertyRenderer::begin(Context* ctx)
	{
		ctx         = ctx ? ctx : static_context();
		bool status = ctx->on_begin_rendering(this);
		m_context_stack.push_back({ctx, ctx->flags(), status});
		return status;
	}

	PropertyRenderer& PropertyRenderer::end()
	{
		ContextEntry& entry = m_context_stack.back();

		entry.context->on_end_rendering(this, entry.active);

		m_context_stack.pop_back();
		return *this;
	}

	void* PropertyRenderer::property_address() const
	{
		return m_event->context;
	}

	Refl::Property* PropertyRenderer::property() const
	{
		return m_event->property;
	}

	bool PropertyRenderer::render_property(void* object, Refl::Property* prop, bool read_only, StringView name)
	{
		if (prop->is_hidden())
			return false;

		Context* ctx = context();

		read_only = read_only || prop->is_read_only();
		ScopedPropID prop_id(object, prop);

		if (auto renderer = ctx->renderer_ptr(prop->refl_class_info()))
		{
			PropertyChangedEvent event(object, prop, m_event);
			m_event = &event;
			m_names_stack.emplace_back(name);

			bool result = (*renderer)(this, prop, read_only);

			m_names_stack.pop_back();
			m_event = event.owner_event;
			return result;
		}

		return false;
	}


	bool PropertyRenderer::render_properties(void* object, Refl::Struct* struct_class, bool read_only, StringView name)
	{
		bool has_changed_props = false;

		StackByteAllocator::Mark mark;

		size_t props_count;
		auto properties = collect_properties(struct_class, props_count);

		if (props_count == 0)
			return true;

		etl::sort(properties, properties + props_count, [&](Refl::Property* first, Refl::Property* second) -> bool {
			const String& group1 = first->group();
			const String& group2 = second->group();

			const bool group1_empty = group1.empty();
			const bool group2_empty = group2.empty();

			if (group1_empty != group2_empty)
				return !group1_empty;

			return group1 < group2;
		});

		for (size_t i = 0; i < props_count; ++i)
		{
			Refl::Property* prop = properties[i];

			bool open = true;
			//const String& group = prop->group();

			// if (!group.empty())
			// {
			// 	open = ctx->on_begin_group(this, group);
			// }

			if (open)
			{
				if (render_property(object, prop, read_only, name))
					has_changed_props = true;
			}

			// if (!group.empty())
			// {
			// 	for (Context* current = ctx; current; current = current->prev())
			// 	{
			// 		current->on_end_group(this, group, open);
			// 		break;
			// 	}
			// }
		}

		return has_changed_props;
	}

	static bool collapsing_header(PropertyRenderer* renderer, const void* id, const char* name);

	class DefaultContext : public PropertyRenderer::Context
	{
	public:
		PropertyRenderingFlags flags() const override { return PropertyRenderingFlags::RenderNames; }

		bool on_begin_rendering(PropertyRenderer* renderer) override
		{
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

		Context& on_end_rendering(PropertyRenderer* renderer, bool status) override
		{
			ImGui::EndTable();
			return *this;
		};

		bool on_begin_group(PropertyRenderer* renderer, const String& group) override
		{
			ImGui::TableNextRow();
			bool result = collapsing_header(renderer, group.c_str(), group.c_str());
			ImGui::Indent();
			return result;
		};

		Context& on_end_group(PropertyRenderer* renderer, const String& group, bool status) override
		{
			ImGui::Unindent();
			return *this;
		};

		uint_t columns() const override { return ImGui::TableGetColumnCount(); }

		Context& column(uint_t index) override
		{
			ImGui::TableSetColumnIndex(index);
			return *this;
		}

		Context& next_row(ImGuiTableRowFlags row_flags = 0, float row_min_height = 0.f) override
		{
			ImGui::TableNextRow(row_flags, row_min_height);
			return *this;
		}

		float cell_width() const override { return ImGui::GetContentRegionAvail().x; }
	};

	PropertyRenderer* PropertyRenderer::static_renderer()
	{
		static PropertyRenderer renderer;
		return &renderer;
	}

	PropertyRenderer::Context* PropertyRenderer::static_context()
	{
		static DefaultContext ctx;
		return &ctx;
	}

	//////////////////////// PROPERTY RENDERERS ////////////////////////

	static void render_name(PropertyRenderer* renderer, Refl::Property* prop)
	{
		auto ctx = renderer->context();

		if (!(renderer->property_rendering_flags() & PropertyRenderingFlags::RenderNames))
		{
			ctx->column(1);
			return;
		}

		ctx->column(0);
		StringView name = renderer->property_name(prop, renderer->property_address());
		ImGui::TextUnformatted(name.data(), name.data() + name.size());

		ctx->column(1);
		auto& tooltip = prop->tooltip();

		if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("%s", tooltip.c_str());
		}
	}

	static void render_name(PropertyRenderer* renderer, StringView name)
	{
		auto ctx = renderer->context();

		if (!(renderer->property_rendering_flags() & PropertyRenderingFlags::RenderNames))
		{
			ctx->column(1);
			return;
		}

		name = renderer->property_name(name);
		ctx->column(0);
		ImGui::TextUnformatted(name.data(), name.data() + name.size());
		ctx->column(1);
	}

	static bool collapsing_header(PropertyRenderer* renderer, const void* id, const char* name)
	{
		::ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiTable* table     = ImGui::GetCurrentContext()->CurrentTable;
		auto ctx              = renderer->context();

		ctx->column(0);
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
		StringView name = renderer->property_name(prop, renderer->property_address());
		return collapsing_header(renderer, prop, name.data());
	}

	static bool render_boolean_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		renderer->context()->next_row();
		render_name(renderer, prop);
		void* context       = renderer->property_address();
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
		PropertyRenderer::Context* ctx = renderer->context();
		byte* address                  = reinterpret_cast<byte*>(prop->address(context));
		ImGuiInputTextFlags flags      = read_only ? ImGuiInputTextFlags_ReadOnly : 0;
		bool changed                   = false;

		if (components == 1)
		{
			ImGui::SetNextItemWidth(ctx->cell_width());
			changed = ImGui::InputScalar("##value", type, address, nullptr, nullptr, nullptr, flags);
		}
		else
		{
			const size_t type_size = ImGui::DataTypeGetInfo(type)->Size;

			static const char* labels[] = {"X", "Y", "Z", "W"};
			static const ImU32 colors[] = {
			        IM_COL32(160, 64, 64, 255), // X — темно-червоний
			        IM_COL32(64, 140, 64, 255), // Y — темно-зелений
			        IM_COL32(64, 108, 176, 255),// Z — темно-синій
			        IM_COL32(140, 120, 64, 255),// W — темно-жовтий
			};

			ImGuiStyle& style   = ImGui::GetStyle();
			const float spacing = style.ItemInnerSpacing.x;

			ImGui::PushMultiItemsWidths(components, ctx->cell_width());

			for (int i = 0; i < components; ++i)
			{
				ImGui::PushID(i);

				if (i > 0)
					ImGui::SameLine(0.f, spacing);

				if (ImGui::InputScalarWithPrefix(labels[i], colors[i], type, address + i * type_size, nullptr, nullptr, nullptr,
				                                 flags))
					changed = true;

				ImGui::PopID();

				ImGui::PopItemWidth();
			}
		}

		if (changed)
			renderer->propagate_property_event(context, prop);

		return changed;
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
		renderer->context()->next_row();
		auto prop = prop_cast_checked<Refl::IntegerProperty>(prop_base);
		render_name(renderer, prop_base);
		return render_scalar_property(renderer->property_address(), prop, renderer, find_imgui_data_type(prop), 1, read_only);
	}

	static bool render_float_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		renderer->context()->next_row();
		render_name(renderer, prop);
		auto float_prop = prop_cast_checked<Refl::FloatProperty>(prop);
		return render_scalar_property(renderer->property_address(), prop, renderer, find_imgui_data_type(float_prop), 1,
		                              read_only);
	}

	static bool render_angle_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		renderer->context()->next_row();
		render_name(renderer, prop);
		auto angle_prop = prop_cast_checked<Refl::AngleProperty>(prop);
		float* angle    = angle_prop->address_as<float>(renderer->property_address());
		float degrees   = Math::degrees(*angle);

		if (ImGui::InputFloat("###Angle", &degrees))
		{
			*angle = Math::radians(degrees);
			renderer->propagate_property_event();
			return true;
		}

		return false;
	}

	static bool render_vector_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop    = prop_cast_checked<Refl::VectorProperty>(prop_base);
		auto element = prop->element_property();

		void* address = renderer->property_address();

		auto render_scalar = [&](ImGuiDataType type) -> bool {
			renderer->context()->next_row();
			render_name(renderer, prop);
			void* scalar_address = prop->address(address);
			return render_scalar_property(scalar_address, element, renderer, type, prop->length(),
			                              read_only || element->is_read_only());
		};

		if (element->is_a<Refl::BooleanProperty>())
		{
			renderer->context()->next_row();
			render_name(renderer, prop);

			bool* data         = prop->address_as<bool>(renderer->property_address());
			const size_t count = prop->length();

			ImGui::PushID(prop);

			for (size_t i = 0; i < count; ++i)
			{
				if (i > 0)
					ImGui::SameLine();

				bool& value = data[i];

				ImVec4 color_on  = ImVec4(0.20f, 0.60f, 0.20f, 1.0f);
				ImVec4 color_off = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);

				ImGui::PushStyleColor(ImGuiCol_Button, value ? color_on : color_off);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, value ? color_on : color_off);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, value ? color_on : color_off);

				char label[16];
				snprintf(label, sizeof(label), "%zu", i);

				if (ImGui::Button(label, ImVec2(24, 24)) && !read_only)
				{
					value = !value;
					renderer->propagate_property_event(&value, element);
					ImGui::PopStyleColor(3);
					ImGui::PopID();
					return true;
				}

				ImGui::PopStyleColor(3);
			}

			ImGui::PopID();
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

		renderer->context()->next_row();

		if (collapsing_header(renderer, prop))
		{
			ImGui::Indent();
			const char* names[] = {"rX", "rY", "rZ", "rW"};

			void* address = renderer->property_address();

			for (size_t i = 0; i < rows; i++)
			{
				void* row_address = prop->row_address(address, i);
				is_changed        = renderer->render_property(row_address, row_prop, read_only, names[i]) || is_changed;
			}
			ImGui::Unindent();
		}

		return is_changed;
	}

	static bool render_quaternion_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop                      = prop_cast_checked<Refl::QuaternionProperty>(prop_base);
		Quaternion* quaternion         = prop->address_as<Quaternion>(renderer->property_address());
		Vector3f degrees               = Math::degrees(Math::euler_angles(*quaternion));
		PropertyRenderer::Context* ctx = renderer->context();

		ctx->next_row();
		render_name(renderer, prop);

		ImGuiInputTextFlags flags = (read_only ? ImGuiInputTextFlags_ReadOnly : 0);

		ImGui::SetNextItemWidth(ctx->cell_width());
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
		auto prop                      = prop_cast_checked<Refl::EnumProperty>(prop_base);
		auto enum_inst                 = prop->enum_instance();
		void* context                  = renderer->property_address();
		PropertyRenderer::Context* ctx = renderer->context();

		ctx->next_row();
		render_name(renderer, prop);

		ImGui::SetNextItemWidth(ctx->cell_width());

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
		PropertyRenderer::Context* ctx = renderer->context();
		ctx->next_row();
		render_name(renderer, prop);

		void* address = renderer->property_address();

		Color* color       = prop->address_as<Color>(address);
		LinearColor linear = {
		        static_cast<float>(color->r) / 255.f,
		        static_cast<float>(color->g) / 255.f,
		        static_cast<float>(color->b) / 255.f,
		        static_cast<float>(color->a) / 255.f,
		};

		uint32_t flags = ImGuiColorEditFlags_Uint8;

		if (read_only)
			flags |= ImGuiColorEditFlags_NoInputs;

		ImGui::SetNextItemWidth(ctx->cell_width());
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
		PropertyRenderer::Context* ctx = renderer->context();
		ctx->next_row();
		render_name(renderer, prop);

		void* context      = renderer->property_address();
		LinearColor* color = prop->address_as<LinearColor>(context);

		uint32_t flags = ImGuiColorEditFlags_Float;

		if (read_only)
			flags |= ImGuiColorEditFlags_NoInputs;

		ImGui::SetNextItemWidth(ctx->cell_width());
		if (ImGui::ColorEdit4("###value", &color->x, flags))
		{
			renderer->propagate_property_event();
			return true;
		}

		return false;
	}

	static bool render_string_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		PropertyRenderer::Context* ctx = renderer->context();
		ctx->next_row();
		render_name(renderer, prop);

		void* address = renderer->property_address();
		if (String* value = prop->address_as<String>(address))
		{
			auto flags = ImGuiInputTextFlags_EnterReturnsTrue | (read_only ? ImGuiInputTextFlags_ReadOnly : 0);

			ImGui::SetNextItemWidth(ctx->cell_width());
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
		renderer->context()->next_row();
		render_name(renderer, prop);

		void* address = renderer->property_address();

		if (Name* value = prop->address_as<Name>(address))
		{
			ImGui::Text("%s", value->c_str());
		}

		return false;
	}

	static bool render_path_property(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)
	{
		renderer->context()->next_row();
		render_name(renderer, prop);

		void* address = renderer->property_address();
		Path* value   = prop->address_as<Path>(address);

		const float size = ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y;

		Any& data = renderer->userdata.get(address);

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
			renderer->context()->column(2);

			if (UI::icon_button(UI::select_icon, "##Select", size))
			{
				Function<void(const Path&)> callback = [renderer, value, &str](const Path& path) {
					*value = path;
					renderer->propagate_property_event();
					str = value->str();
				};

				//imgui_window->widgets.create<ImGuiOpenFile>()->on_select.push(callback);
			}
		}

		return false;
	}

	static bool render_struct_property_internal(PropertyRenderer* renderer, void* struct_address, Refl::Property* prop,
	                                            Refl::Struct* struct_instance, bool read_only)
	{
		bool is_changed   = false;
		bool is_collapsed = false;

		StringView next_name;

		if (prop->is_inline())
		{
			is_collapsed = true;
			next_name    = renderer->property_name();
		}
		else if (prop->is_inline_single_field())
		{
			if (has_only_one_property(struct_instance))
			{
				is_collapsed = true;
				next_name    = renderer->property_name();
			}
			else
			{
				renderer->context()->next_row();
				is_collapsed = collapsing_header(renderer, prop);
			}
		}
		else
		{
			renderer->context()->next_row();
			is_collapsed = collapsing_header(renderer, prop);
		}

		if (is_collapsed)
		{
			push_props_id(struct_address, prop);
			ImGui::Indent();
			is_changed = renderer->render_properties(struct_address, struct_instance, read_only, next_name);
			ImGui::Unindent();
			pop_props_id();
		}

		return is_changed;
	}

	static bool render_object_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		Refl::ObjectProperty* prop = prop_cast_checked<Refl::ObjectProperty>(prop_base);
		void* context              = renderer->property_address();
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
			auto ctx         = renderer->context();
			const float size = ImGui::GetFrameHeight();
			auto object      = prop->object(context);

			bool changed = false;

			ctx->next_row();
			render_name(renderer, prop);
			ctx->column(1);

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
				ctx->column(2);

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
		return render_struct_property_internal(renderer, prop->address(renderer->property_address()), prop,
		                                       prop->struct_instance(), read_only || prop->is_read_only());
	}

	static bool render_array_property(PropertyRenderer* renderer, Refl::Property* prop_base, bool read_only)
	{
		auto prop        = prop_cast_checked<Refl::ArrayProperty>(prop_base);
		bool is_changed  = false;
		const float size = ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y;

		auto ctx      = renderer->context();
		void* address = renderer->property_address();

		renderer->context()->next_row();
		if (ImGui::TableGetColumnCount() > 2)
		{
			ctx->column(2);

			if (!read_only && UI::icon_button(UI::plus_icon, "##emplace_back", size))
			{
				prop->emplace_back(address);
				renderer->propagate_property_event();
				is_changed = true;
			}
		}

		if (collapsing_header(renderer, prop))
		{
			ImGui::Indent();
			Refl::Property* element_prop = prop->element_property();

			size_t count = prop->length(address);

			for (size_t i = 0; i < count; ++i)
			{
				ImGui::PushID(i);

				void* array_object = prop->at(address, i);
				StringView name    = prop->index_name(address, i);

				if (renderer->render_property(array_object, element_prop, element_prop->is_read_only(), name))
				{
					is_changed = true;
				}

				if (ImGui::TableGetColumnCount() > 2)
				{
					ctx->column(2);

					if (!read_only && UI::icon_button(UI::minus_icon, "##erase", size))
					{
						prop->erase(address, i);
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
		renderer->context()->next_row();
		auto prop = prop_cast_checked<Refl::ReflObjectProperty>(prop_base);
		render_name(renderer, prop);

		void* address = renderer->property_address();

		auto current = prop->object(address);
		bool changed = false;

		if (ImGui::BeginCombo("##Combo", current ? current->display_name().c_str() : "None"))
		{
			bool use_none = ImGui::Selectable("None");

			if (use_none || (current = refl_object_property_selector(Refl::Object::static_root(), prop->info())))
			{
				prop->object(address, use_none ? nullptr : current);
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
		renderer->context()->next_row();
		auto prop = prop_cast_checked<Refl::SubClassProperty>(prop_base);
		render_name(renderer, prop);

		void* address = renderer->property_address();
		auto current  = prop->class_instance(address);
		bool changed  = false;

		if (ImGui::BeginCombo("##Combo", current ? current->display_name().c_str() : "None"))
		{
			bool use_none = ImGui::Selectable("None");

			if (use_none || (current = refl_sub_class_property_selector(prop->base_class())))
			{
				prop->class_instance(address, use_none ? nullptr : current);
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

		renderer->context()->next_row();

		if (collapsing_header(renderer, prop))
		{
			void* context   = renderer->property_address();
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
					renderer->context()->next_row();
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
		void* context               = renderer->property_address();

		Any value       = prop->getter(context);
		StringView name = renderer->property_name(prop, context);

		const bool changed = renderer->render_property(value.address(), prop->property(), read_only, name);

		if (changed)
		{
			prop->setter(context, value);
			renderer->propagate_property_event();
		}

		return changed;
	}

	static void on_preinit()
	{
		auto ctx = PropertyRenderer::static_context();

		ctx->renderer<Refl::BooleanProperty>(render_boolean_property);
		ctx->renderer<Refl::IntegerProperty>(render_integer_property);
		ctx->renderer<Refl::FloatProperty>(render_float_property);
		ctx->renderer<Refl::AngleProperty>(render_angle_property);
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
}// namespace Engine::UI
