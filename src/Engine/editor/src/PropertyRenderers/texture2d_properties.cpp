#include <Graphics/imgui.hpp>
#include <Graphics/texture_2D.hpp>
#include <PropertyRenderers/special_renderers.hpp>

namespace Engine
{
    static void renderer(Object* object, Struct* self, bool editable)
    {
        Texture2D* texture = object->instance_cast<Texture2D>();


        static const char* names[] = {
                "None", "BC1", "BC2", "BC3", "BC7",
        };

        int compression = static_cast<int>(texture->image.compression());

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
        bool changed = ImGui::Combo("editor/Compression"_localized, &compression, names, 5);

        if (changed)
        {
            texture->image.recompress(static_cast<ImageCompression>(compression));
            texture->format = texture->image.format();
            texture->postload();
        }
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(Texture2D::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
