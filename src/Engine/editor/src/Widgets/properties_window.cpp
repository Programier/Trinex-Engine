#include <Core/class.hpp>
#include <Core/enum.hpp>
#include <Core/garbage_collector.hpp>>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/properties_window.hpp>
#include <editor_config.hpp>
#include <editor_resources.hpp>
#include <icons.hpp>
#include <imfilebrowser.h>
#include <imgui.h>
#include <imgui_internal.h>


namespace Engine
{
    Map<Struct*, void (*)(class ImGuiObjectProperties*, void*, Struct*, bool)> special_class_properties_renderers;

    static void render_struct_properties(ImGuiObjectProperties*, void* object, class Struct* struct_class, bool editable = true,
                                         bool is_in_table = false);

    static FORCE_INLINE float get_column_width(ImGuiTableColumn& column)
    {
        return column.WorkMaxX - column.WorkMinX;
    }

    static inline bool props_collapsing_header(const char* header_text)
    {
        return ImGuiObjectProperties::collapsing_header("%s", header_text);
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
    }

    static void render_property(ImGuiObjectProperties*, void* object, Property* prop, bool can_edit);

    template<typename Type, PropertyType property_type = PropertyType::Undefined>
    static void render_prop_internal(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit,
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
            }
        }
    }


#define input_text_flags() (editable ? 0 : ImGuiInputTextFlags_ReadOnly)
#define edit_f(type) [](ImGuiObjectProperties * window, void* object, Property* prop, type& value, bool editable) -> bool


    static void render_byte_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<byte>(
                window, object, prop, can_edit, edit_f(byte) {
                    return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_U8, &value, nullptr, nullptr, nullptr,
                                              input_text_flags());
                });
    }

    static void render_signed_byte_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<signed_byte>(
                window, object, prop, can_edit, edit_f(signed_byte) {
                    return ImGui::InputScalar("##Value", ImGuiDataType_S8, &value, nullptr, nullptr, nullptr, input_text_flags());
                });
    }

    static void render_int16_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<int16_t>(
                window, object, prop, can_edit, edit_f(int16_t) {
                    return ImGui::InputScalar("##Value", ImGuiDataType_S16, &value, nullptr, nullptr, nullptr,
                                              input_text_flags());
                });
    }

    static void render_uint16_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<uint16_t>(
                window, object, prop, can_edit, edit_f(uint16_t) {
                    return ImGui::InputScalar("##Value", ImGuiDataType_U16, &value, nullptr, nullptr, nullptr,
                                              input_text_flags());
                });
    }

    static void render_int_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<int_t>(
                window, object, prop, can_edit, edit_f(int_t) {
                    return ImGui::InputScalar("##Value", ImGuiDataType_S32, &value, nullptr, nullptr, nullptr,
                                              input_text_flags());
                });
    }

    static void render_uint_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<uint_t>(
                window, object, prop, can_edit, edit_f(uint_t) {
                    return ImGui::InputScalar("##Value", ImGuiDataType_U32, &value, nullptr, nullptr, nullptr,
                                              input_text_flags());
                });
    }

    static void render_int64_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<int64_t>(
                window, object, prop, can_edit, edit_f(int64_t) {
                    return ImGui::InputScalar("##Value", ImGuiDataType_S64, &value, nullptr, nullptr, nullptr,
                                              input_text_flags());
                });
    }

    static void render_uint64_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<uint64_t>(
                window, object, prop, can_edit, edit_f(uint64_t) {
                    return ImGui::InputScalar("##Value", ImGuiDataType_U64, &value, nullptr, nullptr, nullptr,
                                              input_text_flags());
                });
    }

    static void render_bool_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<bool>(
                window, object, prop, can_edit, edit_f(bool) {
                    if (editable)
                        return ImGui::Checkbox("##Value", &value);
                    bool flag = value;
                    ImGui::Checkbox("##Value", &flag);
                    return false;
                });
    }

    static void render_float_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<float>(
                window, object, prop, can_edit,
                edit_f(float) { return ImGui::InputFloat("##Value", &value, 0.0f, 0.0f, "%.2f", input_text_flags()); });
    }

    static void render_vec2_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector2D>(
                window, object, prop, can_edit,
                edit_f(Vector2D) { return ImGui::InputFloat2("##Value", &value.x, "%.2f", input_text_flags()); });
    }

    static void render_vec3_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector3D>(
                window, object, prop, can_edit,
                edit_f(Vector3D) { return ImGui::InputFloat3("##Value", &value.x, "%.2f", input_text_flags()); });
    }

    static void render_vec4_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector4D>(
                window, object, prop, can_edit,
                edit_f(Vector4D) { return ImGui::InputFloat4("##Value", &value.x, "%.2f", input_text_flags()); });
    }

    static void render_color3_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Color3, PropertyType::Color3>(
                window, object, prop, can_edit, edit_f(Color3) {
                    return ImGui::ColorEdit3("##Value", const_cast<float*>(&value.x),
                                             editable ? 0 : ImGuiColorEditFlags_NoInputs);
                });
    }

    static void render_color4_prop(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Color4, PropertyType::Color4>(
                window, object, prop, can_edit, edit_f(Color4) {
                    return ImGui::ColorEdit4("##Value", const_cast<float*>(&value.x),
                                             editable ? 0 : ImGuiColorEditFlags_NoInputs);
                });
    }

    static void render_string_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<String, PropertyType::String>(
                window, object, prop, can_edit, edit_f(String) {
                    return ImGuiRenderer::InputText("##Value", value, editable ? 0 : ImGuiInputTextFlags_ReadOnly);
                });
    }

    static void render_path_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Path, PropertyType::Path>(
                window, object, prop, can_edit, edit_f(Path) {
                    ImGuiRenderer::Window* imgui_window = ImGuiRenderer::Window::current();

                    const char* text = value.empty() ? "None" : value.c_str();
                    if (ImGui::Selectable(text) && editable)
                    {
                        Function<void(const Path&)> callback = [object, prop](const Path& path) {
                            prop->property_value(object, path);
                        };
                        imgui_window->window_list.create<ImGuiOpenFile>()->on_select.push(callback);
                    }

                    return false;
                });
    }


    static const char* enum_element_name(void* userdata, int index)
    {
        return reinterpret_cast<Enum*>(userdata)->entries()[index].name.c_str();
    }

    static void render_enum_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        PropertyValue value = prop->property_value(object);
        if (!value.has_value())
            return;

        Enum* enum_class = prop->enum_instance();
        if (!enum_class)
            return;

        EnumerateType current            = value.cast<EnumerateType>();
        const Enum::Entry* current_entry = enum_class->entry(current);

        if (!current_entry)
            return;


        int index                          = static_cast<int>(current_entry->index);
        const Vector<Enum::Entry>& entries = enum_class->entries();

        render_prop_name(prop);

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::Combo("##ComboValue", &index, enum_element_name, enum_class, entries.size()) && can_edit)
        {
            current_entry = &entries[index];
            value         = PropertyValue(current_entry->value, PropertyType::Enum);
            prop->property_value(object, value);
        }
    }

    static void render_object_property(ImGuiObjectProperties* window, Object* object, Property* prop, bool can_edit)
    {
        PropertyValue value = prop->property_value(object);

        if (value.has_value())
        {
            object = value.cast<Object*>();

            if (object)
            {
                Struct* struct_class = object->class_instance();
                if (props_collapsing_header(prop->name().c_str()))
                {
                    push_props_id(object, prop);
                    ImGui::Indent(editor_config.collapsing_indent);
                    render_struct_properties(window, object, struct_class, can_edit, true);
                    ImGui::Unindent(editor_config.collapsing_indent);
                    pop_props_id();
                }
            }
        }
    }

    static bool render_object_reference_internal(ImGuiObjectProperties* window, void* object, Property* prop, Object*& value,
                                                 bool can_edit)
    {
        Struct* self     = prop->struct_instance();
        const float size = ImGui::GetFrameHeight();

        bool changed = false;

        {
            ImGui::TableSetColumnIndex(1);

            ImGui::PushID("##Image");
            ImGui::Image(Icons::find_imgui_icon(value), {100, 100});

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

    static void render_object_reference(ImGuiObjectProperties* window, Object* object, Property* prop, bool can_edit)
    {
        return render_prop_internal<Object*, PropertyType::ObjectReference>(window, object, prop, can_edit,
                                                                            render_object_reference_internal);
    }


    static void render_struct_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        PropertyValue value = prop->property_value(object);

        if (value.has_value())
        {
            void* struct_object  = value.cast<void*>();
            Struct* struct_class = prop->struct_instance();
            if (props_collapsing_header(prop->name().c_str()))
            {
                push_props_id(object, prop);
                ImGui::Indent(editor_config.collapsing_indent);
                render_struct_properties(window, struct_object, struct_class, can_edit, true);
                ImGui::Unindent(editor_config.collapsing_indent);
                pop_props_id();
            }
        }
    }

    static void render_array_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        PropertyValue value = prop->property_value(object);
        if (!value.has_value())
            return;

        ImGui::TableSetColumnIndex(2);
        ArrayPropertyInterface* interface = reinterpret_cast<ArrayPropertyInterface*>(prop);
        const float size                  = ImGui::GetFrameHeight();

        if (can_edit && ImGui::Button("+", {size, size}))
        {
            interface->emplace_back(object);
        }

        if (props_collapsing_header(prop->name().c_str()))
        {
            ImGui::Indent(editor_config.collapsing_indent);
            Property* element_property = interface->element_type();

            size_t count = interface->elements_count(object);
            auto name    = element_property->name();
            static Vector<Name> names;


            for (size_t i = 0; i < count;)
            {
                ImGui::TableNextRow();
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
                render_property(window, array_object, element_property, true);

                ++i;
                ImGui::PopID();
            }

            element_property->name(name);
            ImGui::Unindent(editor_config.collapsing_indent);
        }
    }

    static void render_property(ImGuiObjectProperties* window, void* object, Property* prop, bool can_edit)
    {
        can_edit = can_edit && !prop->is_const();
        push_props_id(object, prop);

        switch (prop->type())
        {
            case PropertyType::Byte:
                render_byte_prop(window, object, prop, can_edit);
                break;
            case PropertyType::SignedByte:
                render_signed_byte_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Int16:
                render_int16_prop(window, object, prop, can_edit);
                break;
            case PropertyType::UnsignedInt16:
                render_uint16_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Int:
                render_int_prop(window, object, prop, can_edit);
                break;
            case PropertyType::UnsignedInt:
                render_uint_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Int64:
                render_int64_prop(window, object, prop, can_edit);
                break;
            case PropertyType::UnsignedInt64:
                render_uint64_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Bool:
                render_bool_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Float:
                render_float_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Vec2:
                render_vec2_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Vec3:
                render_vec3_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Vec4:
                render_vec4_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Color3:
                render_color3_prop(window, object, prop, can_edit);
                break;
            case PropertyType::Color4:
                render_color4_prop(window, object, prop, can_edit);
                break;
            case PropertyType::String:
                render_string_property(window, object, prop, can_edit);
                break;
            case PropertyType::Path:
                render_path_property(window, object, prop, can_edit);
                break;
            case PropertyType::Enum:
                render_enum_property(window, object, prop, can_edit);
                break;
            case PropertyType::Object:
                render_object_property(window, reinterpret_cast<Object*>(object), prop, can_edit);
                break;
            case PropertyType::ObjectReference:
                render_object_reference(window, reinterpret_cast<Object*>(object), prop, can_edit);
                break;
            case PropertyType::Struct:
                render_struct_property(window, object, prop, can_edit);
                break;

            case PropertyType::Array:
                render_array_property(window, object, prop, can_edit);
                break;
            default:
                break;
        }

        pop_props_id();
    }

    static void render_struct_properties(ImGuiObjectProperties* window, void* object, class Struct* struct_class, bool editable,
                                         bool is_in_table)
    {
        if (!is_in_table)
            begin_prop_table();


        for (Struct* self = struct_class; self; self = self->parent())
        {
            auto it = special_class_properties_renderers.find(self);

            if (it != special_class_properties_renderers.end())
            {
                it->second(window, object, struct_class, editable);
            }
        }

        for (auto& [group, props] : window->properties_map(struct_class))
        {
            bool open = true;

            if (group != Name::none)
            {
                open = props_collapsing_header(group.c_str());
                ImGui::Indent(editor_config.collapsing_indent);
            }

            if (open)
            {
                for (auto& prop : props)
                {
                    ImGui::TableNextRow();
                    render_property(window, object, prop, editable);
                }
            }

            if (group != Name::none)
            {
                ImGui::Unindent(editor_config.collapsing_indent);
            }
        }

        if (!is_in_table)
            end_prop_table();
    }

    ImGuiObjectProperties::ImGuiObjectProperties()
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
            ImGui::Text("editor/Class: %s"_localized, m_object->class_instance()->name().c_str());
            if (ImGui::Button("editor/Apply changes"_localized))
            {
                m_object->apply_changes();
            }
            ImGui::Separator();
            ::Engine::render_struct_properties(this, m_object, m_object->class_instance(), true, false);
        }
        ImGui::End();

        return open;
    }

    Struct* ImGuiObjectProperties::struct_instance() const
    {
        return m_object->class_instance();
    }

    Object* ImGuiObjectProperties::object() const
    {
        return m_object;
    }


    ImGuiObjectProperties::PropertiesMap& ImGuiObjectProperties::build_props_map(Struct* self)
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
        return *this;
    }

    const ImGuiObjectProperties::PropertiesMap& ImGuiObjectProperties::properties_map(Struct* self)
    {
        auto& map = m_properties[self];

        if (map.empty() && !self->properties().empty())
        {
            return build_props_map(self);
        }

        return map;
    }

    void ImGuiObjectProperties::render_struct_properties(void* object, class Struct* struct_class, bool editable)
    {
        ::Engine::render_struct_properties(this, object, struct_class, editable, true);
    }

    bool ImGuiObjectProperties::collapsing_header(const char* format, ...)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGuiTable* table   = ImGui::GetCurrentContext()->CurrentTable;

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
        bool result = ImGui::TreeNodeExV("##NodeId", ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_CollapsingHeader,
                                         format, args);
        va_end(args);

        window->ClipRect       = clip_rect;
        window->ParentWorkRect = parent_work_rect;
        return result;
    }


    const char* ImGuiObjectProperties::name()
    {
        return "editor/Properties Title"_localized;
    }
}// namespace Engine
