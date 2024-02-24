#include <Graphics/imgui.hpp>
#include <Graphics/texture_2D.hpp>
#include <PropertyRenderers/special_renderers.hpp>

namespace Engine
{
    static void renderer(Object* object, Struct* self, bool editable)
    {
        Texture2D* texture = object->instance_cast<Texture2D>();
        bool is_compressed = texture->image.is_compressed();

        if (is_compressed)
        {
            ImGui::Text("Texture is compressed!");
        }
        else
        {
            if (ImGui::Button("Compress"))
            {
                texture->image.compress();
                texture->format = texture->image.format();
                texture->postload();
            }
        }
    }

    static void initialize_special_class_properties_renderers()
    {
        special_class_properties_renderers[reinterpret_cast<Struct*>(Texture2D::static_class_instance())] = renderer;
    }

    static PostInitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
