#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <DefaultResources/default.hpp>

namespace Engine
{
    namespace DefaultResources
    {

        namespace Samplers
        {
            ENGINE_EXPORT Sampler* default_sampler = nullptr;
        }

        namespace Textures
        {
            ENGINE_EXPORT Texture2D* default_texture = nullptr;
        }

        namespace Buffers
        {
            ENGINE_EXPORT PositionVertexBuffer* screen_position = nullptr;
        }

        namespace Materials
        {
            ENGINE_EXPORT Material* sprite            = nullptr;
            ENGINE_EXPORT Material* screen            = nullptr;
            ENGINE_EXPORT Material* base_pass         = nullptr;
            ENGINE_EXPORT Material* batched_lines     = nullptr;
            ENGINE_EXPORT Material* batched_triangles = nullptr;
            ENGINE_EXPORT Material* point_light       = nullptr;
            ENGINE_EXPORT Material* spot_light        = nullptr;
            ENGINE_EXPORT Material* directional_light = nullptr;
            ENGINE_EXPORT Material* ambient_light     = nullptr;
            ENGINE_EXPORT Material* imgui             = nullptr;
        }// namespace Materials

        namespace Meshes
        {
            ENGINE_EXPORT StaticMesh* cube     = nullptr;
            ENGINE_EXPORT StaticMesh* sphere   = nullptr;
            ENGINE_EXPORT StaticMesh* cylinder = nullptr;
        }// namespace Meshes

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
    DefaultResources::group_name::object = reinterpret_cast<class_name*>(                                                        \
            load_object_from_memory(name##_data, name##_len, "DefaultPackage::" #group_name "::" #name));                        \
    reinterpret_cast<Object*>(DefaultResources::group_name::object)->add_reference()


    void load_default_resources()
    {
        load_default_asset(DefaultSampler, default_sampler, Sampler, Samplers);
        load_default_asset(DefaultTexture, default_texture, Texture2D, Textures);
        load_default_asset(ScreenPositionBuffer, screen_position, PositionVertexBuffer, Buffers);
        load_default_asset(SpriteMaterial, sprite, Material, Materials);
        load_default_asset(ScreenMaterial, screen, Material, Materials);
        load_default_asset(BasePassMaterial, base_pass, Material, Materials);
        load_default_asset(BatchedLinesMaterial, batched_lines, Material, Materials);
        load_default_asset(BatchedTrianglesMaterial, batched_triangles, Material, Materials);
        load_default_asset(PointLightMaterial, point_light, Material, Materials);
        load_default_asset(SpotLightMaterial, spot_light, Material, Materials);
        load_default_asset(DirectionalLightMaterial, directional_light, Material, Materials);
        load_default_asset(AmbientLightMaterial, ambient_light, Material, Materials);
        load_default_asset(ImGuiMaterial, imgui, Material, Materials);
        load_default_asset(Cube, cube, StaticMesh, Meshes);
        load_default_asset(Sphere, sphere, StaticMesh, Meshes);
        load_default_asset(Cylinder, cylinder, StaticMesh, Meshes);
    }
}// namespace Engine
