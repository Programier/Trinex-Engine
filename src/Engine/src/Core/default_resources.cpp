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
        ENGINE_EXPORT Material* base_color_to_screen_mat           = nullptr;
        ENGINE_EXPORT Material* default_material                   = nullptr;
        ENGINE_EXPORT Material* gbuffer_lines_material             = nullptr;
        ENGINE_EXPORT Material* scene_output_lines_material        = nullptr;
        ENGINE_EXPORT Material* point_light_material               = nullptr;
    }// namespace DefaultResources

    ENGINE_EXPORT Object* load_object_from_memory(const byte* data, size_t size, const StringView& name, class Package* package)
    {
        if (size > 0)
        {
            Vector<byte> tmp(data, data + size);
            VectorReader reader = &tmp;
            if (Object* object = Object::load_object(&reader))
            {
                if (!name.empty())
                {
                    object->name(name);
                }

                if (package)
                {
                    package->add_object(object);
                }

                return object;
            }
        }

        return nullptr;
    }

#define load_default_asset(name, object, class_name)                                                                             \
    DefaultResources::object = reinterpret_cast<class_name*>(load_object_from_memory(name##_data, name##_len, #name, package))

    void EngineInstance::load_default_resources()
    {
        Package* package = Package::find_package("DefaultPackage", true);
        load_default_asset(DefaultSampler, default_sampler, Sampler);
        load_default_asset(DefaultTexture, default_texture, Texture2D);
        load_default_asset(SpriteMaterial, sprite_material, Material);
        load_default_asset(ScreenPositionBuffer, screen_position_buffer, PositionVertexBuffer);

        load_default_asset(BaseColorToScreenMat, base_color_to_screen_mat, Material);
        load_default_asset(DefaultMaterial, default_material, Material);
        load_default_asset(GBufferLinesMat, gbuffer_lines_material, Material);
        load_default_asset(SceneOutputLinesMat, scene_output_lines_material, Material);
        load_default_asset(PointLightMaterial, point_light_material, Material);

        DefaultResourcesInitializeController().execute();
        m_flags(DefaultResourcesInitTriggered, true);
    }
}// namespace Engine
