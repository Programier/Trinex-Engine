#pragma once
#include <Core/math/box.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	class MaterialInterface;

	struct ENGINE_EXPORT MeshSurface {
		trinex_declare_struct(MeshSurface, void);

		uint32_t base_vertex_index;
		uint32_t first_index;
		uint32_t vertices_count;
		uint16_t material_index;

		bool serialize(Archive& ar);
	};

	class ENGINE_EXPORT StaticMesh : public Object
	{
		trinex_declare_class(StaticMesh, Object);

	public:
		struct ENGINE_EXPORT LOD {
			trinex_declare_struct(LOD, void);

			Vector<PositionVertexBuffer> positions;
			Vector<TexCoordVertexBuffer> tex_coords;
			Vector<ColorVertexBuffer> colors;
			Vector<NormalVertexBuffer> normals;
			Vector<TangentVertexBuffer> tangents;

			IndexBuffer indices;
			Vector<MeshSurface> surfaces;

		private:
			PositionVertexBuffer* find_position_buffer(Index index);
			TexCoordVertexBuffer* find_tex_coord_buffer(Index index);
			ColorVertexBuffer* find_color_buffer(Index index);
			NormalVertexBuffer* find_normal_buffer(Index index);
			TangentVertexBuffer* find_tangent_buffer(Index index);

		public:
			VertexBufferBase* find_vertex_buffer(RHIVertexSemantic semantic, Index index = 0);
			size_t vertex_count() const;
			size_t indices_count() const;
			bool serialize(Archive& ar);
		};

		Vector<MaterialInterface*> materials;
		Box3f bounds;
		Vector<LOD> lods;

		StaticMesh& init_render_resources();
		StaticMesh& apply_changes() override;
		bool serialize(Archive& ar) override;
		StaticMesh& postload() override;
	};

	class ENGINE_EXPORT SkeletalMesh : public Object
	{
		trinex_declare_class(SkeletalMesh, Object);

	public:
		struct ENGINE_EXPORT LOD {
			trinex_declare_struct(LOD, void);

			Vector<PositionVertexBuffer> positions;
			Vector<TexCoordVertexBuffer> tex_coords;
			Vector<ColorVertexBuffer> colors;
			Vector<NormalVertexBuffer> normals;
			Vector<TangentVertexBuffer> tangents;
			Vector<BlendWeightVertexBuffer> blend_weights;
			Vector<BlendIndicesVertexBuffer> blend_indices;

			IndexBuffer indices;
			Vector<MeshSurface> surfaces;

		private:
			VertexBufferBase* find_position_buffer(Index index);
			VertexBufferBase* find_tex_coord_buffer(Index index);
			VertexBufferBase* find_color_buffer(Index index);
			VertexBufferBase* find_normal_buffer(Index index);
			VertexBufferBase* find_tangent_buffer(Index index);
			VertexBufferBase* find_blend_weights_buffer(Index index);
			VertexBufferBase* find_blend_indices_buffer(Index index);

		public:
			VertexBufferBase* find_vertex_buffer(RHIVertexSemantic semantic, Index index = 0);
			size_t vertex_count() const;
			size_t indices_count() const;
			bool serialize(Archive& ar);
		};

		Vector<MaterialInterface*> materials;
		Box3f bounds = {};
		Vector<LOD> lods;
		uint32_t bones = 0;

		SkeletalMesh& init_render_resources();
		SkeletalMesh& apply_changes() override;
		bool serialize(Archive& ar) override;
		SkeletalMesh& postload() override;
	};
}// namespace Engine
