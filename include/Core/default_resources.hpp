#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>

namespace Engine
{
	class Sampler;
	class Texture;
	class Material;
	class Texture2D;
	class Texture3D;
	class TextureCube;
	class PositionVertexBuffer;
	class StaticMesh;

	namespace DefaultResources
	{
		namespace Textures
		{
			ENGINE_EXPORT extern Texture2D* white;
			ENGINE_EXPORT extern Texture2D* black;
			ENGINE_EXPORT extern Texture2D* gray;
			ENGINE_EXPORT extern Texture2D* normal;
			ENGINE_EXPORT extern Texture2D* default_texture;
			ENGINE_EXPORT extern TextureCube* default_texture_cube;
			ENGINE_EXPORT extern Texture2D* noise4x4;
			ENGINE_EXPORT extern Texture2D* noise16x16;
			ENGINE_EXPORT extern Texture2D* noise128x128;
			ENGINE_EXPORT extern Texture3D* default_lut;
		}// namespace Textures

		namespace Buffers
		{
			ENGINE_EXPORT extern PositionVertexBuffer* screen_quad;
		}

		namespace Materials
		{
			ENGINE_EXPORT extern Material* sprite;
			ENGINE_EXPORT extern Material* base_pass;
		}// namespace Materials

		namespace Meshes
		{
			ENGINE_EXPORT extern StaticMesh* cube;
			ENGINE_EXPORT extern StaticMesh* sphere;
			ENGINE_EXPORT extern StaticMesh* cylinder;
			ENGINE_EXPORT extern StaticMesh* plane;
			ENGINE_EXPORT extern StaticMesh* cone;
		}// namespace Meshes

	}// namespace DefaultResources

	ENGINE_EXPORT class Object* load_object_from_memory(const byte* data, size_t size, const StringView& fullname);
}// namespace Engine
