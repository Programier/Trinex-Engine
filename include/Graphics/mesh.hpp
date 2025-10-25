#pragma once
#include <Core/etl/flat_set.hpp>
#include <Core/math/box.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	class MaterialInterface;

	struct ENGINE_EXPORT MeshSurface {
		trinex_declare_struct(MeshSurface, void);

		struct Attribute {
			RHIVertexSemantic semantic;
			RHIVertexFormat format;
			byte stream;
			byte offset;

			inline bool operator<(const Attribute& attribute) const { return semantic < attribute.semantic; }
		};

		FlatSet<Attribute> attributes;
		RHIPrimitiveTopology topology = RHIPrimitiveTopology::TriangleList;
		RHIIndexFormat index_format   = RHIIndexFormat::Undefined;
		uint32_t first_vertex         = 0;
		uint32_t first_index          = ~0U;
		uint32_t vertices_count       = 0;
		uint16_t material_index       = 0;

		bool serialize(Archive& ar);

		inline bool is_indexed() const { return first_index != ~0U; }

		inline const Attribute* find_attribute(RHIVertexSemantic semantic) const
		{
			Attribute attribute;
			attribute.semantic = semantic;
			auto it            = attributes.find(attribute);

			if (it == attributes.end())
				return nullptr;
			return &(*it);
		}
	};

	class ENGINE_EXPORT StaticMesh : public Object
	{
		trinex_declare_class(StaticMesh, Object);

	public:
		struct ENGINE_EXPORT LOD {
			trinex_declare_struct(LOD, void);

			Vector<MeshSurface> surfaces;
			Vector<VertexBufferBase> buffers;
			IndexBuffer indices;

		public:
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
