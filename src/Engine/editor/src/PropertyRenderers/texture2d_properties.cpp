#include <Graphics/imgui.hpp>
#include <Graphics/texture_2D.hpp>
#include <PropertyRenderers/special_renderers.hpp>
#include <Core/enum.hpp>

namespace Engine
{
    static const char* format_to_string(const ColorFormat& format)
    {
        static Enum* format_enum = Enum::static_find("Engine::ColorFormat", true);
        return format_enum->entry(static_cast<EnumerateType>(format))->name.c_str();
    }

    // static void renderer(class ImGuiObjectProperties* window, void* object, Struct* self, bool editable)
    // {
    //     Texture2D* texture  = reinterpret_cast<Texture2D*>(object);
    //     bool image_is_empty = texture->image.empty();

    //     bool is_compressed = image_is_empty ? false : texture->image.is_compressed();
    //     ColorFormat format = image_is_empty ? texture->format : texture->image.format();

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::Text("%s", "editor/Format"_localized);
    //     ImGui::TableSetColumnIndex(1);
    //     ImGui::Text("%s", format_to_string(format));

    //     if (!is_compressed && !image_is_empty)
    //     {
    //         ImGui::TableSetColumnIndex(2);
    //         if (ImGui::SmallButton("editor/Compress"_localized))
    //         {
    //             texture->image.compress();
    //             texture->format = texture->image.format();
    //             texture->postload();
    //         }
    //     }

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::Text("%s", "editor/Buffer size"_localized);
    //     ImGui::TableSetColumnIndex(1);
    //     ImGui::Text("%zu KB", texture->image.buffer().size() / 1024);
    // }

    // static void initialize_special_class_properties_renderers()
    // {
    //     special_class_properties_renderers[reinterpret_cast<Struct*>(Texture2D::static_class_instance())] = renderer;
    // }

    //static InitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
