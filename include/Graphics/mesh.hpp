#pragma once
#include <Core/asset.hpp>
#include <Core/etl/flat_set.hpp>
#include <Core/math/box.hpp>
#include <Core/pointer.hpp>
#include <Graphics/gpu_buffers.hpp>

#include <Graphics/mesh.generated.hpp>

namespace Trinex
{
	class MaterialInterface;

	struct MeshVertexAttribute
	{
		RHISemantic semantic;
		RHIVertexFormat format;
		u8 stream;
		u8 offset;

		inline bool operator<(const MeshVertexAttribute& attribute) const { return semantic < attribute.semantic; }
	};

	trinex_reflect()
	struct trinex_api MeshSurface
	{
		trinex_reflection_body();

		trinex_property(IsReadOnly, IsTransient)
		RHITopology topology = RHITopology::TriangleList;

		trinex_property(IsReadOnly, IsTransient)
		u32 first_vertex = 0;

		trinex_property(IsReadOnly, IsTransient)
		u32 first_index = ~0U;

		trinex_property(IsReadOnly, IsTransient)
		u32 vertices_count = 0;

		trinex_property(IsReadOnly, IsTransient)
		u16 material_index = 0;

		bool serialize(Archive& ar);
		inline bool is_indexed() const { return first_index != ~0U; }
	};

	using MeshVertexStream = Vector3f;

	struct MeshSurfaceStream
	{
		Vector2f16 uv0;
		Vector2f16 uv1;

		u32 normal;
		u32 tangent;
	};

	struct MeshAnimationStream
	{
		u8 indices[4];
		u8 weights[4];
	};

	class trinex_api StaticMesh : public Asset
	{
		trinex_class(StaticMesh, Asset);

	public:
		trinex_reflect()
		struct trinex_api LOD
		{
			//trinex_reflection_body();

			VertexBuffer<MeshVertexStream> vertex_stream;
			VertexBuffer<MeshSurfaceStream> surface_stream;

			IndexBuffer indices;

			trinex_property()
			Array<MeshSurface> surfaces;

			bool serialize(Archive& ar);
		};

		Array<MaterialInterface*> materials;
		Box3f bounds;
		Array<LOD> lods;

		StaticMesh& rebuild() override;
		bool serialize(Archive& ar) override;
	};

	class trinex_api SkeletalMesh : public Asset
	{
		trinex_class(SkeletalMesh, Asset);

	public:
		struct trinex_api LOD
		{
			trinex_old_struct(LOD, void);
			VertexBuffer<MeshVertexStream> vertex_stream;
			VertexBuffer<MeshSurfaceStream> surface_stream;
			VertexBuffer<MeshAnimationStream> animation_stream;

			IndexBuffer indices;
			Array<MeshSurface> surfaces;

			bool serialize(Archive& ar);
		};

		Array<MaterialInterface*> materials;
		Box3f bounds = {};
		Array<LOD> lods;
		u32 bones = 0;

		SkeletalMesh& rebuild() override;
		bool serialize(Archive& ar) override;
	};
} // namespace Trinex
