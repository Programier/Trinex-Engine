#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class Sampler;
	class Texture;
	class Material;
	class Texture2D;
	class PositionVertexBuffer;
	class StaticMesh;

	namespace DefaultResources
	{
		namespace Samplers
		{
			ENGINE_EXPORT extern Sampler* default_sampler;
		}

		namespace Textures
		{
			ENGINE_EXPORT extern Texture2D* default_texture;
		}

		namespace Buffers
		{
			ENGINE_EXPORT extern PositionVertexBuffer* screen_position;
		}

		namespace Materials
		{
			ENGINE_EXPORT extern Material* sprite;
			ENGINE_EXPORT extern Material* screen;
			ENGINE_EXPORT extern Material* base_pass;
			ENGINE_EXPORT extern Material* batched_lines;
			ENGINE_EXPORT extern Material* batched_triangles;
			ENGINE_EXPORT extern Material* point_light;
			ENGINE_EXPORT extern Material* spot_light;
			ENGINE_EXPORT extern Material* directional_light;
			ENGINE_EXPORT extern Material* ambient_light;
		}// namespace Materials

		namespace Meshes
		{
			ENGINE_EXPORT extern StaticMesh* cube;
			ENGINE_EXPORT extern StaticMesh* sphere;
			ENGINE_EXPORT extern StaticMesh* cylinder;
		}// namespace Meshes

	}// namespace DefaultResources

	ENGINE_EXPORT class Object* load_object_from_memory(const byte* data, size_t size, const StringView& fullname);
}// namespace Engine
