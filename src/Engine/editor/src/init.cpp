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

    namespace EditorResources
    {
        Sampler* default_sampler = nullptr;
    }

    static void resource_loading()
    {
#define load_resource(name, type) reinterpret_cast<type*>(load_object_from_memory(name##_data, name##_len, "Editor::" #name))
        load_object_from_memory(DefaultIcon_data, DefaultIcon_len, "Editor::DefaultIcon");
        load_object_from_memory(AddIcon_data, AddIcon_len, "Editor::AddIcon");
        load_object_from_memory(MoveIcon_data, MoveIcon_len, "Editor::MoveIcon");
        load_object_from_memory(RemoveIcon_data, RemoveIcon_len, "Editor::RemoveIcon");
        load_object_from_memory(RotateIcon_data, RotateIcon_len, "Editor::RotateIcon");
        load_object_from_memory(ScaleIcon_data, ScaleIcon_len, "Editor::ScaleIcon");
        load_object_from_memory(SelectIcon_data, SelectIcon_len, "Editor::SelectIcon");
        load_object_from_memory(MoreIcon_data, MoreIcon_len, "Editor::MoreIcon");
        load_object_from_memory(PointLightSprite_data, PointLightSprite_len, "Editor::PointLightSprite");

        EditorResources::default_sampler = load_resource(DefaultSampler, Sampler);
        //        load_object_from_memory(AxisMaterial_data, AxisMaterial_len, "Editor::AxisMaterial");
        load_object_from_memory(GridMaterial_data, GridMaterial_len, "Editor::GridMaterial");
        Icons::on_editor_package_loaded();
    }

    static DefaultResourcesInitializeController on_init(resource_loading, "Load Editor Package");
}// namespace Engine
