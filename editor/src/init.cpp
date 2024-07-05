#include "DefaultResources/editor.hpp"
#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Graphics/pipeline_buffers.hpp>

namespace Engine
{
    static void skip_destroy_assets(Object* object)
    {
        if (object->class_instance()->is_asset())
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
        Texture2D* default_icon                             = nullptr;
        Texture2D* add_icon                                 = nullptr;
        Texture2D* move_icon                                = nullptr;
        Texture2D* remove_icon                              = nullptr;
        Texture2D* rotate_icon                              = nullptr;
        Texture2D* scale_icon                               = nullptr;
        Texture2D* select_icon                              = nullptr;
        Texture2D* more_icon                                = nullptr;
        Texture2D* light_sprite                             = nullptr;
        Texture2D* blueprint_texture                        = nullptr;
        Sampler* default_sampler                            = nullptr;
        Material* axis_material                             = nullptr;
        Material* grid_material                             = nullptr;
        Material* point_light_overlay_material              = nullptr;
        Material* spot_light_overlay_material               = nullptr;
        PositionVertexBuffer* spot_light_overlay_positions  = nullptr;
        PositionVertexBuffer* point_light_overlay_positions = nullptr;
    }// namespace EditorResources


    static void create_circle(const Function<void(float, float)>& callback)
    {
        for (int i = 1; i <= 360; ++i)
        {
            for (int j = -1; j <= 1; j++)
            {
                float angle = glm::two_pi<float>() * static_cast<float>(i + j) / 360.f;
                float x     = glm::cos(angle);
                float z     = glm::sin(angle);
                callback(x, z);
            }
        }
    }

    static void create_spot_light_overlay_positions()
    {
        EditorResources::spot_light_overlay_positions = Object::new_instance<EngineResource<PositionVertexBuffer>>();
        auto buffer                                   = EditorResources::spot_light_overlay_positions;

        static constexpr float circle_y = -1.f;

        // Create circle
        create_circle([buffer](float x, float z) { buffer->buffer.push_back(Vector3D(x, circle_y, z)); });

        buffer->buffer.push_back({0, 0, 0});
        buffer->buffer.push_back({1, circle_y, 0});

        buffer->buffer.push_back({0, 0, 0});
        buffer->buffer.push_back({-1, circle_y, 0});

        buffer->buffer.push_back({0, 0, 0});
        buffer->buffer.push_back({0, circle_y, 1});

        buffer->buffer.push_back({0, 0, 0});
        buffer->buffer.push_back({0, circle_y, -1});
        buffer->init_resource();
    }

    static void create_point_light_overlay_positions()
    {
        EditorResources::point_light_overlay_positions = Object::new_instance<EngineResource<PositionVertexBuffer>>();
        auto buffer                                    = EditorResources::point_light_overlay_positions;

        create_circle([buffer](float y, float z) { buffer->buffer.push_back(Vector3D(0, y, z)); });
        create_circle([buffer](float x, float z) { buffer->buffer.push_back(Vector3D(x, 0, z)); });
        create_circle([buffer](float x, float y) { buffer->buffer.push_back(Vector3D(x, y, 0)); });
        buffer->init_resource();
    }

    static void initialialize_editor()
    {
#define load_resource(var, name, type, group_name)                                                                               \
    EditorResources::var =                                                                                                       \
            reinterpret_cast<type*>(load_object_from_memory(name##_data, name##_len, "Editor::" #group_name "::" #name));        \
    reinterpret_cast<Object*>(EditorResources::var)->add_reference()

        load_resource(default_icon, DefaultIcon, Texture2D, Textures);
        load_resource(add_icon, AddIcon, Texture2D, Textures);
        load_resource(move_icon, MoveIcon, Texture2D, Textures);
        load_resource(remove_icon, RemoveIcon, Texture2D, Textures);
        load_resource(rotate_icon, RotateIcon, Texture2D, Textures);
        load_resource(scale_icon, ScaleIcon, Texture2D, Textures);
        load_resource(select_icon, SelectIcon, Texture2D, Textures);
        load_resource(more_icon, MoreIcon, Texture2D, Textures);
        load_resource(blueprint_texture, BlueprintBackground, Texture2D, Textures);
        load_resource(light_sprite, PointLightSprite, Texture2D, Textures);
        load_resource(default_sampler, DefaultSampler, Sampler, Samplers);
        load_resource(axis_material, AxisMaterial, Material, Materials);
        load_resource(grid_material, GridMaterial, Material, Materials);
        load_resource(point_light_overlay_material, PointLightOverlay, Material, Materials);
        load_resource(spot_light_overlay_material, SpotLightOverlay, Material, Materials);

        create_point_light_overlay_positions();
        create_spot_light_overlay_positions();

        Icons::on_editor_package_loaded();
        GarbageCollector::on_unreachable_check.push(skip_destroy_assets);
    }

    static DefaultResourcesInitializeController on_init(initialialize_editor, "Load Editor Package");
}// namespace Engine
