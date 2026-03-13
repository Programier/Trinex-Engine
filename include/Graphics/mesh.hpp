#pragma once
#include <Core/etl/flat_set.hpp>
#include <Core/math/box.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Trinex
{
	class MaterialInterface;

	struct MeshVertexAttribute {
		RHIVertexSemantic semantic;
		RHIVertexFormat format;
		u8 stream;
		u8 offset;

		inline bool operator<(const MeshVertexAttribute& attribute) const { return semantic < attribute.semantic; }
	};

	struct ENGINE_EXPORT MeshSurface {
		trinex_struct(MeshSurface, void);

		RHITopology topology = RHITopology::TriangleList;
		u32 first_vertex              = 0;
		u32 first_index               = ~0U;
		u32 vertices_count            = 0;
		u16 material_index            = 0;

		bool serialize(Archive& ar);

		inline bool is_indexed() const { return first_index != ~0U; }
	};

	class ENGINE_EXPORT StaticMesh : public Object
	{
		trinex_class(StaticMesh, Object);

	public:
		struct ENGINE_EXPORT LOD {
			trinex_struct(LOD, void);

			Vector<MeshSurface> surfaces;
			FlatSet<MeshVertexAttribute> attributes;
			Vector<VertexBufferBase> buffers;
			IndexBuffer indices;

		public:
			bool serialize(Archive& ar);

			inline const MeshVertexAttribute* find_attribute(RHIVertexSemantic semantic) const
			{
				MeshVertexAttribute attribute;
				attribute.semantic = semantic;
				auto it            = attributes.find(attribute);

				if (it == attributes.end())
					return nullptr;
				return &(*it);
			}
		};

		Vector<MaterialInterface*> materials;
		Box3f bounds;
		Vector<LOD> lods;

		StaticMesh& init_render_resources();
		bool serialize(Archive& ar) override;
		StaticMesh& postload() override;
	};

	class ENGINE_EXPORT SkeletalMesh : public Object
	{
		trinex_class(SkeletalMesh, Object);

	public:
		struct ENGINE_EXPORT LOD {
			trinex_struct(LOD, void);

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
			VertexBufferBase* find_position_buffer(usize index);
			VertexBufferBase* find_tex_coord_buffer(usize index);
			VertexBufferBase* find_color_buffer(usize index);
			VertexBufferBase* find_normal_buffer(usize index);
			VertexBufferBase* find_tangent_buffer(usize index);
			VertexBufferBase* find_blend_weights_buffer(usize index);
			VertexBufferBase* find_blend_indices_buffer(usize index);

		public:
			VertexBufferBase* find_vertex_buffer(RHIVertexSemantic semantic, usize index = 0);
			usize vertex_count() const;
			usize indices_count() const;
			bool serialize(Archive& ar);
		};

		Vector<MaterialInterface*> materials;
		Box3f bounds = {};
		Vector<LOD> lods;
		u32 bones = 0;

		SkeletalMesh& init_render_resources();
		bool serialize(Archive& ar) override;
		SkeletalMesh& postload() override;
	};
}// namespace Trinex
