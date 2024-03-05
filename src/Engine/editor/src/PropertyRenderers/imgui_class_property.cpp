#include <Core/class.hpp>
#include <Core/enum.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <PropertyRenderers/imgui_class_property.hpp>
#include <Widgets/imgui_windows.hpp>
#include <imfilebrowser.h>
#include <imgui.h>

namespace Engine
{
    static constexpr float indent = 5.f;

    Map<Struct*, void (*)(void*, Struct*, bool)> special_class_properties_renderers;

    class Object;

    static void render_prop_internal(void* object, Struct* self, bool editable)
    {
        bool is_not_first = false;
        for (auto& [name, properties] : self->grouped_properties())
        {
            if (is_not_first)
            {
                ImGui::NewLine();
            }
            else
            {
                is_not_first = true;
            }

            bool is_empty_group = name.to_string().empty();
            if (!is_empty_group)
            {
                ImGui::PushID(name.c_str());
                ImGui::Indent(indent);
            }

            if (is_empty_group || ImGui::CollapsingHeader(name.c_str()))
            {
                ImGui::Indent(indent);
                for (Property* prop : properties)
                {
                    render_property(object, prop, editable);
                }
                ImGui::Unindent(indent);
            }

            if (!is_empty_group)
            {
                ImGui::Unindent(indent);
                ImGui::PopID();
            }
        }
    }


    template<typename... Args>
    static void prop_text(const char* format, Args... args)
    {
        ImGui::Text(format, args...);
    }

    template<typename Type, PropertyType property_type = PropertyType::Undefined>
    static void render_prop_internal(void* object, Property* prop, bool can_edit, void (*show_only)(Property*, const Type&),
                                     bool (*edit)(Property*, Type&))
    {
        if (prop->is_const() || !can_edit)
        {
            PropertyValue result = prop->property_value(object);
            if (result.has_value())
            {
                Type value = result.cast<Type>();
                show_only(prop, value);
            }
        }
        else
        {
            PropertyValue result = prop->property_value(object);
            if (result.has_value())
            {
                Type value = result.cast<Type>();
                if (edit(prop, value))
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
    }

#define view_f(type) [](Property * prop, const type& value)
#define edit_f(type) [](Property * prop, type & value) -> bool

    static void render_byte_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<byte>(
                object, prop, can_edit, view_f(byte) { prop_text("%s: %hhu", prop->name().c_str(), value); },
                edit_f(byte) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_U8, &value); });
    }

    static void render_signed_byte_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<signed_byte>(
                object, prop, can_edit, view_f(signed_byte) { prop_text("%s: %hhd", prop->name().c_str(), value); },
                edit_f(signed_byte) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_S8, &value); });
    }


    static void render_int16_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<int16_t>(
                object, prop, can_edit, view_f(int16_t) { prop_text("%s: %hd", prop->name().c_str(), value); },
                edit_f(int16_t) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_S16, &value); });
    }

    static void render_uint16_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<uint16_t>(
                object, prop, can_edit, view_f(uint16_t) { prop_text("%s: %hu", prop->name().c_str(), value); },
                edit_f(uint16_t) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_U16, &value); });
    }

    static void render_int_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<int_t>(
                object, prop, can_edit, view_f(int_t) { prop_text("%s: %d", prop->name().c_str(), int_t(value)); },
                edit_f(int_t) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_S32, &value); });
    }

    static void render_uint_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<uint_t>(
                object, prop, can_edit, view_f(uint_t) { prop_text("%s: %d", prop->name().c_str(), int_t(value)); },
                edit_f(uint_t) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_U32, &value); });
    }

    static void render_int64_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<int64_t>(
                object, prop, can_edit, view_f(int64_t) { prop_text("%s: %zd", prop->name().c_str(), value); },
                edit_f(int64_t) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_S64, &value); });
    }

    static void render_uint64_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<uint64_t>(
                object, prop, can_edit, view_f(uint64_t) { prop_text("%s: %zu", prop->name().c_str(), value); },
                edit_f(uint64_t) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_U64, &value); });
    }

    static void render_bool_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<bool>(
                object, prop, can_edit, view_f(bool) { prop_text("%s: %s", prop->name().c_str(), value ? "True" : "False"); },
                edit_f(bool) { return ImGui::Checkbox(prop->name().c_str(), &value); });
    }

    static void render_float_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<float>(
                object, prop, can_edit, view_f(float) { prop_text("%s: %.2f", prop->name().c_str(), value); },
                edit_f(float) { return ImGui::InputFloat(prop->name().c_str(), &value, 0.0f, 0.0f, "%.2f"); });
    }

    static void render_vec2_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector2D>(
                object, prop, can_edit,
                view_f(Vector2D) { prop_text("%s: {%.2f, %.2f}", prop->name().c_str(), value.x, value.y); },
                edit_f(Vector2D) { return ImGui::InputFloat2(prop->name().c_str(), &value.x, "%.2f"); });
    }

    static void render_vec3_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector3D>(
                object, prop, can_edit,
                view_f(Vector3D) { prop_text("%s: {%.2f, %.2f, %.2f}", prop->name().c_str(), value.x, value.y, value.z); },
                edit_f(Vector3D) { return ImGui::InputFloat3(prop->name().c_str(), &value.x, "%.2f"); });
    }

    static void render_vec4_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector4D>(
                object, prop, can_edit,
                view_f(Vector4D) {
                    prop_text("%s: {%.2f, %.2f, %.2f, %.2f}", prop->name().c_str(), value.x, value.y, value.z, value.w);
                },
                edit_f(Vector4D) { return ImGui::InputFloat4(prop->name().c_str(), &value.x, "%.2f"); });
    }

    static void render_color3_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Color3, PropertyType::Color3>(
                object, prop, can_edit,
                view_f(Color3) {
                    ImGui::ColorEdit3(prop->name().c_str(), const_cast<float*>(&value.x), ImGuiColorEditFlags_NoInputs);
                },
                edit_f(Color3) { return ImGui::ColorEdit3(prop->name().c_str(), const_cast<float*>(&value.x)); });
    }

    static void render_color4_prop(void* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Color4, PropertyType::Color4>(
                object, prop, can_edit,
                view_f(Color4) {
                    ImGui::ColorEdit4(prop->name().c_str(), const_cast<float*>(&value.x), ImGuiColorEditFlags_NoInputs);
                },
                edit_f(Color4) { return ImGui::ColorEdit4(prop->name().c_str(), const_cast<float*>(&value.x)); });
    }

    static void render_path_property(void* object, Property* prop, bool can_edit)
    {
        ImGuiRenderer::Window* window = ImGuiRenderer::Window::current();

        PropertyValue value = prop->property_value(object);
        if (!value.has_value())
            return;

        Path path = value.cast<const Path&>();

        if (ImGui::Selectable(Strings::format("{}: {}", prop->name().c_str(), path.str().c_str()).c_str()))
        {
            Function<void(const Path&)> callback = [object, prop](const Path& path) { prop->property_value(object, path); };
            window->window_list.create<ImGuiOpenFile>()->on_select.push(callback);
        }
    }


    static const char* enum_element_name(void* userdata, int index)
    {
        return reinterpret_cast<Enum*>(userdata)->entries()[index].name.c_str();
    }

    static void render_enum_property(void* object, Property* prop, bool can_edit)
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

        if (can_edit && !prop->is_const())
        {
            int index                          = static_cast<int>(current_entry->index);
            const Vector<Enum::Entry>& entries = enum_class->entries();

            if (ImGui::Combo(prop->name().c_str(), &index, enum_element_name, enum_class, entries.size()))
            {
                current_entry = &entries[index];
                value         = PropertyValue(current_entry->value, PropertyType::Enum);
                prop->property_value(object, value);
            }
        }
        else
        {
            prop_text("%s: %s", prop->name().c_str(), current_entry->name.c_str());
        }
    }

    static void render_object_property(Object* object, Property* prop, bool can_edit)
    {
        PropertyValue value = prop->property_value(object);
        if (value.has_value())
        {
            object = value.cast<Object*>();
            if (object)
            {
                render_object_properties(object, can_edit);
            }
            else
            {
                ImGui::Text("%s: No Object", prop->name().c_str());
            }
        }
    }

    static void render_object_reference(Object* object, Property* prop, bool can_edit)
    {
        render_object_property(object, prop, can_edit);
        Struct* self = prop->struct_instance();

        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContendBrowser->Object");
            if (payload)
            {
                IM_ASSERT(payload->DataSize == sizeof(Object*));

                Object* new_object = *reinterpret_cast<Object**>(payload->Data);

                if (new_object && new_object->class_instance()->is_a(self))
                {
                    PropertyValue value = PropertyValue(new_object, PropertyType::ObjectReference);
                    prop->property_value(object, value);
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    static void render_struct_property(void* object, Property* prop, bool can_edit)
    {
        PropertyValue value = prop->property_value(object);
        if (value.has_value())
        {
            void* struct_object  = value.cast<void*>();
            Struct* struct_class = prop->struct_instance();

            ImGui::PushID(prop->name().c_str());
            if (ImGui::CollapsingHeader(prop->name().c_str()))
            {
                ImGui::Indent(indent);
                render_prop_internal(struct_object, struct_class, can_edit);
                ImGui::Unindent(indent);
            }
            ImGui::PopID();
        }
    }

    static void render_array_property(void* object, Property* prop, bool can_edit)
    {
        ArrayPropertyInterface* interface = reinterpret_cast<ArrayPropertyInterface*>(prop);
        Property* element_property        = interface->element_type();
        size_t count                      = interface->elements_count(object);

        ImGui::PushID(prop->name().c_str());

        if (ImGui::CollapsingHeader(prop->name().c_str()))
        {
            ImGui::Indent(indent);
            for (size_t i = 0; i < count; i++)
            {
                void* array_object = interface->at(object, i);
                ImGui::PushID(i);
                ImGui::Text("[%zu]", i);
                ImGui::SameLine();

                ImGui::BeginGroup();
                render_property(array_object, element_property, can_edit);
                ImGui::EndGroup();

                ImGui::PopID();
            }
            ImGui::Unindent(indent);
        }

        ImGui::PopID();
    }

    void render_property(void* object, Property* prop, bool can_edit)
    {
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.50f);
        switch (prop->type())
        {
            case PropertyType::Byte:
                render_byte_prop(object, prop, can_edit);
                break;
            case PropertyType::SignedByte:
                render_signed_byte_prop(object, prop, can_edit);
                break;
            case PropertyType::Int16:
                render_int16_prop(object, prop, can_edit);
                break;
            case PropertyType::UnsignedInt16:
                render_uint16_prop(object, prop, can_edit);
                break;
            case PropertyType::Int:
                render_int_prop(object, prop, can_edit);
                break;
            case PropertyType::UnsignedInt:
                render_uint_prop(object, prop, can_edit);
                break;
            case PropertyType::Int64:
                render_int64_prop(object, prop, can_edit);
                break;
            case PropertyType::UnsignedInt64:
                render_uint64_prop(object, prop, can_edit);
                break;
            case PropertyType::Bool:
                render_bool_prop(object, prop, can_edit);
                break;
            case PropertyType::Float:
                render_float_prop(object, prop, can_edit);
                break;
            case PropertyType::Vec2:
                render_vec2_prop(object, prop, can_edit);
                break;
            case PropertyType::Vec3:
                render_vec3_prop(object, prop, can_edit);
                break;
            case PropertyType::Vec4:
                render_vec4_prop(object, prop, can_edit);
                break;
            case PropertyType::Color3:
                render_color3_prop(object, prop, can_edit);
                break;
            case PropertyType::Color4:
                render_color4_prop(object, prop, can_edit);
                break;
            case PropertyType::Path:
                render_path_property(object, prop, can_edit);
                break;
            case PropertyType::Enum:
                render_enum_property(object, prop, can_edit);
                break;
            case PropertyType::Object:
                render_object_property(reinterpret_cast<Object*>(object), prop, can_edit);
                break;
            case PropertyType::ObjectReference:
                render_object_reference(reinterpret_cast<Object*>(object), prop, can_edit);
                break;
            case PropertyType::Struct:
                render_struct_property(object, prop, can_edit);
                break;

            case PropertyType::Array:
                render_array_property(object, prop, can_edit);
                break;
            default:
                break;
        }

        ImGui::PopItemWidth();
    }

    void render_struct_properties(void* object, class Struct* struct_class, bool editable)
    {

        struct StructInfo {
            Struct* self                           = nullptr;
            void (*renderer)(void*, Struct*, bool) = nullptr;
            bool has_properties                    = false;

            bool is_visible() const
            {
                return has_properties || renderer;
            }
        };

        static Vector<Vector<StructInfo>> infos_stack;
        static Index stack_index = 0;

        if (stack_index >= infos_stack.size())
        {
            infos_stack.emplace_back();
        }

        Index current_stack_index = stack_index++;

        Index count         = 0;
        Index visible_count = 0;

        for (Struct* self = struct_class; self; self = self->parent())
        {
            if (infos_stack[current_stack_index].size() <= count)
            {
                infos_stack[current_stack_index].emplace_back();
            }

            auto& info          = infos_stack[current_stack_index][count];
            info.self           = self;
            info.has_properties = !self->properties().empty();

            auto it       = special_class_properties_renderers.find(self);
            info.renderer = it != special_class_properties_renderers.end() ? it->second : nullptr;

            if (info.is_visible())
            {
                ++visible_count;
            }
            ++count;
        }

        ImGui::BeginGroup();
        for (Index i = 0; i < count; ++i)
        {
            StructInfo info = infos_stack[current_stack_index][i];

            if (info.is_visible())
            {
                ImGui::PushID(info.self->base_name_splitted().c_str());
                bool is_open = true;

                if (visible_count > 1)
                {
                    is_open = ImGui::CollapsingHeader(info.self->base_name_splitted().c_str());
                }

                if (is_open)
                {
                    if (info.has_properties)
                    {
                        render_prop_internal(object, info.self, editable);
                    }

                    if (info.renderer)
                    {
                        ImGui::Indent(indent);
                        info.renderer(object, info.self, editable);
                        ImGui::Separator();
                        ImGui::Unindent(indent);
                    }
                }


                ImGui::PopID();
            }
        }

        ImGui::EndGroup();

        --stack_index;
    }

    void render_object_properties(class Object* object, bool editable)
    {
        render_struct_properties(object, object->class_instance(), editable);
    }
}// namespace Engine
