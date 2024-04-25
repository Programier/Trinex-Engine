#include "DefaultResources/editor.hpp"
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/class.hpp>

namespace Engine
{
    static void skip_destroy_assets(Object* object)
    {
        if(object->class_instance()->is_asset())
        {
            object->flags(Object::Flag::IsUnreachable, false);
        }
    }

    namespace Icons
    {
        extern void on_editor_package_loaded();
    }

    namespace EditorResources
    {
        Texture2D* default_icon       = nullptr;
        Texture2D* add_icon           = nullptr;
        Texture2D* move_icon          = nullptr;
        Texture2D* remove_icon        = nullptr;
        Texture2D* rotate_icon        = nullptr;
        Texture2D* scale_icon         = nullptr;
        Texture2D* select_icon        = nullptr;
        Texture2D* more_icon          = nullptr;
        Texture2D* point_light_sprite = nullptr;
        Sampler* default_sampler      = nullptr;
        Material* axis_material       = nullptr;
        Material* grid_material       = nullptr;
    }// namespace EditorResources

    static void resource_loading()
    {
#define load_resource(var, name, type)                                                                                           \
    EditorResources::var = reinterpret_cast<type*>(load_object_from_memory(name##_data, name##_len, "Editor::" #name));          \
    reinterpret_cast<Object*>(EditorResources::var)->add_reference()

        load_resource(default_icon, DefaultIcon, Texture2D);
        load_resource(add_icon, AddIcon, Texture2D);
        load_resource(move_icon, MoveIcon, Texture2D);
        load_resource(remove_icon, RemoveIcon, Texture2D);
        load_resource(rotate_icon, RotateIcon, Texture2D);
        load_resource(scale_icon, ScaleIcon, Texture2D);
        load_resource(select_icon, SelectIcon, Texture2D);
        load_resource(more_icon, MoreIcon, Texture2D);
        load_resource(point_light_sprite, PointLightSprite, Texture2D);
        load_resource(default_sampler, DefaultSampler, Sampler);
        load_resource(axis_material, AxisMaterial, Material);
        load_resource(grid_material, GridMaterial, Material);

        Icons::on_editor_package_loaded();

        GarbageCollector::on_unreachable_check.push(skip_destroy_assets);
    }

    static DefaultResourcesInitializeController on_init(resource_loading, "Load Editor Package");
}// namespace Engine
