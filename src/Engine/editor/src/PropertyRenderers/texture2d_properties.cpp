#include <Graphics/imgui.hpp>
#include <Graphics/texture_2D.hpp>
#include <PropertyRenderers/special_renderers.hpp>

namespace Engine
{
    static const char* format_to_string(const ColorFormat& format)
    {
        switch (format)
        {
            case ColorFormat::R8Unorm:
                return "R8 Unorm";
            case ColorFormat::R8G8Unorm:
                return "R8G8 Unorm";
            case ColorFormat::R8G8B8Unorm:
                return "R8G8B8 Unorm";
            case ColorFormat::R8G8B8A8Unorm:
                return "R8G8B8A8 Unorm";
            case ColorFormat::BC1Unorm:
                return "BC1 Unorm";
            case ColorFormat::BC3Unorm:
                return "BC3 Unorm";

            default:
                return "Internal format";
        }
    }

    static void renderer(void* object, Struct* self, bool editable)
    {
        Texture2D* texture  = reinterpret_cast<Texture2D*>(object);
        bool image_is_empty = texture->image.empty();

        bool is_compressed = image_is_empty ? false : texture->image.is_compressed();
        ColorFormat format = image_is_empty ? texture->format : texture->image.format();

        ImGui::Text("editor/Format: %s"_localized, format_to_string(format));

        if(!is_compressed && !image_is_empty)
        {
            ImGui::SameLine();

            if(ImGui::SmallButton("editor/Compress"_localized))
            {
                texture->image.compress();
                texture->format = texture->image.format();
                texture->postload();
            }
        }

        ImGui::Text("editor/Buffer size: %zu KB"_localized, texture->image.buffer().size() / 1024);
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(Texture2D::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
