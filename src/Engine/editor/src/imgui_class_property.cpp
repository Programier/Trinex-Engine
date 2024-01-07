#include <Core/class.hpp>
#include <Core/enum.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <imfilebrowser.h>
#include <imgui.h>
#include <imgui_class_property.hpp>
#include <imgui_windows.hpp>

namespace Engine
{
    class Object;

    template<typename... Args>
    static void prop_text(const char* format, Args... args)
    {
        ImGui::Text(format, args...);
    }

    template<typename Type>
    static void render_prop_internal(Object* object, Property* prop, bool can_edit, void (*show_only)(Property*, const Type&),
                                     bool (*edit)(Property*, Type&))
    {
        if (prop->is_const() || !can_edit)
        {
            PropertyValue result = prop->property_value(object);
            if (result.has_value())
            {
                Type value = std::any_cast<Type>(result);
                show_only(prop, value);
            }
        }
        else
        {
            PropertyValue result = prop->property_value(object);
            if (result.has_value())
            {
                Type value = std::any_cast<Type>(result);
                if (edit(prop, value))
                {
                    result = value;
                    prop->property_value(object, result);
                }
            }
        }
    }

#define view_f(type) [](Property * prop, const type& value)
#define edit_f(type) [](Property * prop, type & value) -> bool

    static void render_byte_prop(Object* object, Property* prop, bool can_edit)
    {
        render_prop_internal<byte>(
                object, prop, can_edit, view_f(byte) { prop_text("%s: %d", prop->name().c_str(), int(value)); },
                edit_f(byte) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_U8, &value); });
    }

    static void render_int_prop(Object* object, Property* prop, bool can_edit)
    {
        render_prop_internal<int_t>(
                object, prop, can_edit, view_f(int_t) { prop_text("%s: %d", prop->name().c_str(), int_t(value)); },
                edit_f(int_t) { return ImGui::InputScalar(prop->name().c_str(), ImGuiDataType_S32, &value); });
    }

    static void render_bool_prop(Object* object, Property* prop, bool can_edit)
    {
        render_prop_internal<bool>(
                object, prop, can_edit, view_f(bool) { prop_text("%s: %s", prop->name().c_str(), value ? "True" : "False"); },
                edit_f(bool) { return ImGui::Checkbox(prop->name().c_str(), &value); });
    }

    static void render_float_prop(Object* object, Property* prop, bool can_edit)
    {
        render_prop_internal<float>(
                object, prop, can_edit, view_f(float) { prop_text("%s: %.2f", prop->name().c_str(), value); },
                edit_f(float) { return ImGui::InputFloat(prop->name().c_str(), &value); });
    }

    static void render_vec2_prop(Object* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector2D>(
                object, prop, can_edit,
                view_f(Vector2D) { prop_text("%s: {%.2f, %.2f}", prop->name().c_str(), value.x, value.y); },
                edit_f(Vector2D) { return ImGui::InputFloat2(prop->name().c_str(), &value.x); });
    }

    static void render_vec3_prop(Object* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector3D>(
                object, prop, can_edit,
                view_f(Vector3D) { prop_text("%s: {%.2f, %.2f, %.2f}", prop->name().c_str(), value.x, value.y, value.z); },
                edit_f(Vector3D) { return ImGui::InputFloat3(prop->name().c_str(), &value.x); });
    }

    static void render_vec4_prop(Object* object, Property* prop, bool can_edit)
    {
        render_prop_internal<Vector4D>(
                object, prop, can_edit,
                view_f(Vector4D) {
                    prop_text("%s: {%.2f, %.2f, %.2f, %.2f}", prop->name().c_str(), value.x, value.y, value.z, value.w);
                },
                edit_f(Vector4D) { return ImGui::InputFloat4(prop->name().c_str(), &value.x); });
    }

    static void render_path_property(Object* object, Property* prop, bool can_edit)
    {
        ImGuiRenderer::Window* window = ImGuiRenderer::Window::current();

        PropertyValue value = prop->property_value(object);
        if (!value.has_value())
            return;

        Path path = std::any_cast<Path>(value);

        if (ImGui::Selectable(Strings::format("{}: {}", prop->name().c_str(), path.c_str()).c_str()))
        {
            Function<void(Package*, const Path&)> callback = [object, prop](Package*, const Path& path) {
                prop->property_value(object, FS::relative(path));
            };

            window->window_list.create<ImGuiOpenFile>(nullptr, callback);
        }
    }


    const char* enum_element_name(void* userdata, int index)
    {
        return reinterpret_cast<Enum*>(userdata)->entries()[index].name.c_str();
    }

    static void render_enum_property(Object* object, Property* prop, bool can_edit)
    {
        PropertyValue value = prop->property_value(object);
        if (!value.has_value())
            return;

        Enum* enum_class = reinterpret_cast<Enum*>(prop->property_class());
        if (!enum_class)
            return;

        EnumerateType current            = std::any_cast<EnumerateType>(value);
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
                value         = current_entry->value;
                prop->property_value(object, value);
            }
        }
        else
        {
            prop_text("%s: %s", prop->name().c_str(), current_entry->name.c_str());
        }
    }

    void render_object_property(Object* object, Property* prop, bool can_edit)
    {
        PropertyValue value = prop->property_value(object);
        if (value.has_value())
        {
            object = std::any_cast<Object*>(value);
            render_object_properties(object, can_edit);
        }
    }

    static void process_property(Object* object, Property* prop, bool can_edit)
    {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2.f);
        switch (prop->type())
        {
            case Property::Type::Byte:
                render_byte_prop(object, prop, can_edit);
                break;
            case Property::Type::Int:
                render_int_prop(object, prop, can_edit);
                break;
            case Property::Type::Bool:
                render_bool_prop(object, prop, can_edit);
                break;
            case Property::Type::Float:
                render_float_prop(object, prop, can_edit);
                break;
            case Property::Type::Vec2:
                render_vec2_prop(object, prop, can_edit);
                break;
            case Property::Type::Vec3:
                render_vec3_prop(object, prop, can_edit);
                break;
            case Property::Type::Vec4:
                render_vec4_prop(object, prop, can_edit);
                break;
            case Property::Type::Path:
                render_path_property(object, prop, can_edit);
                break;
            case Property::Type::Enum:
                render_enum_property(object, prop, can_edit);
                break;
            case Property::Type::Object:
                render_object_property(object, prop, can_edit);
                break;
            default:
                break;
        }
    }

    void render_object_properties(class Object* object, bool editable)
    {
        for (Class* self = object->class_instance(); self; self = self->parent())
        {
            if (!self->properties().empty())
            {
                if (ImGui::CollapsingHeader(self->name().c_str()))
                {
                    for (auto& [name, properties] : self->grouped_properties())
                    {
                        bool is_empty_group = name.to_string().empty();
                        if (!is_empty_group)
                            ImGui::Indent(5.f);

                        if (is_empty_group || ImGui::CollapsingHeader(name.c_str()))
                        {
                            ImGui::Indent(5.f);
                            for (Property* prop : properties)
                            {
                                process_property(object, prop, editable);
                            }
                            ImGui::Unindent(5.f);
                        }

                        if (!is_empty_group)
                            ImGui::Unindent(5.f);
                    }
                }
            }
        }
    }
}// namespace Engine
