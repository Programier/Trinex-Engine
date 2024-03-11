#include "DefaultResources/editor.hpp"
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>

namespace Engine
{
    namespace Icons
    {
        extern void on_editor_package_loaded();
    }

    static void resource_loading()
    {
        Package* editor = Package::find_package("Editor", true);
        load_object_from_memory(DefaultIcon_data, DefaultIcon_len, "DefaultIcon", editor);
        load_object_from_memory(AddIcon_data, AddIcon_len, "AddIcon", editor);
        load_object_from_memory(MoveIcon_data, MoveIcon_len, "MoveIcon", editor);
        load_object_from_memory(RemoveIcon_data, RemoveIcon_len, "RemoveIcon", editor);
        load_object_from_memory(RotateIcon_data, RotateIcon_len, "RotateIcon", editor);
        load_object_from_memory(ScaleIcon_data, ScaleIcon_len, "ScaleIcon", editor);
        load_object_from_memory(SelectIcon_data, SelectIcon_len, "SelectIcon", editor);
        load_object_from_memory(MoreIcon_data, MoreIcon_len, "MoreIcon", editor);
        load_object_from_memory(PointLightSprite_data, PointLightSprite_len, "PointLightSprite", editor);

        load_object_from_memory(DefaultSampler_data, DefaultSampler_len, "DefaultSampler", editor);
//        load_object_from_memory(AxisMaterial_data, AxisMaterial_len, "AxisMaterial", editor);
//        load_object_from_memory(GridMaterial_data, GridMaterial_len, "GridMaterial", editor);
        Icons::on_editor_package_loaded();
    }

    static DefaultResourcesInitializeController on_init(resource_loading, "Load Editor Package");
}// namespace Engine
