#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
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
			trinex_api extern Texture2D* white;
			trinex_api extern Texture2D* black;
			trinex_api extern Texture2D* gray;
			trinex_api extern Texture2D* normal;
			trinex_api extern Texture2D* default_texture;
			trinex_api extern TextureCube* default_texture_cube;
			trinex_api extern Texture2D* noise4x4;
			trinex_api extern Texture2D* noise16x16;
			trinex_api extern Texture2D* noise128x128;
			trinex_api extern Texture3D* default_lut;
		} // namespace Textures

		namespace Buffers
		{
			trinex_api extern PositionVertexBuffer* screen_quad;
		}

		namespace Materials
		{
			trinex_api extern Material* sprite;
			trinex_api extern Material* base_pass;
		} // namespace Materials

		namespace Meshes
		{
			trinex_api extern StaticMesh* cube;
			trinex_api extern StaticMesh* sphere;
			trinex_api extern StaticMesh* cylinder;
			trinex_api extern StaticMesh* plane;
			trinex_api extern StaticMesh* cone;
		} // namespace Meshes

	} // namespace DefaultResources

	trinex_api class DeprecatedObject* load_object_from_memory(const u8* data, usize size, const StringView& fullname);
} // namespace Trinex
