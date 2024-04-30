#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <DefaultResources/default.hpp>

namespace Engine
{
    namespace DefaultResources
    {
        ENGINE_EXPORT Sampler* default_sampler                     = nullptr;
        ENGINE_EXPORT Texture2D* default_texture                   = nullptr;
        ENGINE_EXPORT Material* sprite_material                    = nullptr;
        ENGINE_EXPORT PositionVertexBuffer* screen_position_buffer = nullptr;
        ENGINE_EXPORT Material* screen_material                    = nullptr;
        ENGINE_EXPORT Material* default_material                   = nullptr;
        ENGINE_EXPORT Material* batched_lines_material             = nullptr;
        ENGINE_EXPORT Material* batched_triangles_material         = nullptr;
        ENGINE_EXPORT Material* point_light_material               = nullptr;
        ENGINE_EXPORT Material* spot_light_material                = nullptr;
        ENGINE_EXPORT Material* directional_light_material         = nullptr;
        ENGINE_EXPORT Material* ambient_light_material             = nullptr;
    }// namespace DefaultResources

    ENGINE_EXPORT Object* load_object_from_memory(const byte* data, size_t size, const StringView& name)
    {
        if (size > 0)
        {
            Vector<byte> tmp(data, data + size);
            VectorReader reader = &tmp;
            return Object::load_object(name, &reader);
        }

        return nullptr;
    }

#define load_default_asset(name, object, class_name, group_name)                                                                 \
    DefaultResources::object = reinterpret_cast<class_name*>(                                                                    \
            load_object_from_memory(name##_data, name##_len, "DefaultPackage::" #group_name "::" #name));                        \
    reinterpret_cast<Object*>(DefaultResources::object)->add_reference()


    void EngineInstance::load_default_resources()
    {
        load_default_asset(DefaultSampler, default_sampler, Sampler, Samplers);
        load_default_asset(DefaultTexture, default_texture, Texture2D, Textures);
        load_default_asset(ScreenPositionBuffer, screen_position_buffer, PositionVertexBuffer, Buffers);
        load_default_asset(SpriteMaterial, sprite_material, Material, Materials);
        load_default_asset(ScreenMaterial, screen_material, Material, Materials);
        load_default_asset(DefaultMaterial, default_material, Material, Materials);
        load_default_asset(BatchedLinesMaterial, batched_lines_material, Material, Materials);
        load_default_asset(BatchedTrianglesMaterial, batched_triangles_material, Material, Materials);
        load_default_asset(PointLightMaterial, point_light_material, Material, Materials);
        load_default_asset(SpotLightMaterial, spot_light_material, Material, Materials);
        load_default_asset(DirectionalLightMaterial, directional_light_material, Material, Materials);
        load_default_asset(AmbientLightMaterial, ambient_light_material, Material, Materials);

        DefaultResourcesInitializeController().execute();
        m_flags(DefaultResourcesInitTriggered, true);
    }
}// namespace Engine
