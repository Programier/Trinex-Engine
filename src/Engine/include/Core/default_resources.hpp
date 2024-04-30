#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class Sampler;
    class Texture;
    class Material;
    class Texture2D;
    class PositionVertexBuffer;

    namespace DefaultResources
    {
        ENGINE_EXPORT extern Sampler* default_sampler;
        ENGINE_EXPORT extern Texture2D* default_texture;
        ENGINE_EXPORT extern Material* sprite_material;
        ENGINE_EXPORT extern PositionVertexBuffer* screen_position_buffer;
        ENGINE_EXPORT extern Material* screen_material;
        ENGINE_EXPORT extern Material* default_material;
        ENGINE_EXPORT extern Material* gbuffer_lines_material;
        ENGINE_EXPORT extern Material* scene_output_lines_material;
        ENGINE_EXPORT extern Material* point_light_material;
        ENGINE_EXPORT extern Material* spot_light_material;
        ENGINE_EXPORT extern Material* directional_light_material;
        ENGINE_EXPORT extern Material* ambient_light_material;
    }// namespace DefaultResources

    ENGINE_EXPORT class Object* load_object_from_memory(const byte* data, size_t size, const StringView& fullname);
}// namespace Engine
