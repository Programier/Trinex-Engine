#pragma once
#include <Core/enums.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Engine/aabb.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	class MaterialInterface;

	struct ENGINE_EXPORT MeshMaterial {
		trinex_declare_struct(MeshMaterial, void);

		MaterialInterface* material = nullptr;
		PolicyID policy;
		byte surface_index;
	};

	struct ENGINE_EXPORT MeshSurface {
		uint32_t base_vertex_index;
		uint32_t first_index;
		uint32_t vertices_count;
	};

	class ENGINE_EXPORT StaticMesh : public Object
	{
		trinex_declare_class(StaticMesh, Object);

	public:
		struct ENGINE_EXPORT LOD {
			Vector<PositionVertexBuffer> positions;
			Vector<TexCoordVertexBuffer> tex_coords;
			Vector<ColorVertexBuffer> colors;
			Vector<NormalVertexBuffer> normals;
			Vector<TangentVertexBuffer> tangents;
			Vector<BitangentVertexBuffer> bitangents;

			IndexBuffer indices;
			Vector<MeshSurface> surfaces;

		private:
			PositionVertexBuffer* find_position_buffer(Index index);
			TexCoordVertexBuffer* find_tex_coord_buffer(Index index);
			ColorVertexBuffer* find_color_buffer(Index index);
			NormalVertexBuffer* find_normal_buffer(Index index);
			TangentVertexBuffer* find_tangent_buffer(Index index);
			BitangentVertexBuffer* find_bitangent_buffer(Index index);

		public:
			VertexBufferBase* find_vertex_buffer(VertexBufferSemantic semantic, Index index = 0);
			size_t vertex_count() const;
			size_t indices_count() const;
			bool serialize(Archive& ar);
		};

		Vector<MeshMaterial> materials;
		AABB_3Df bounds;
		Vector<LOD> lods;

		StaticMesh();
		StaticMesh& init_render_resources();
		StaticMesh& apply_changes() override;
		bool serialize(Archive& ar) override;
		StaticMesh& postload() override;
	};

	class ENGINE_EXPORT DynamicMesh : public Object
	{
		trinex_declare_class(DynamicMesh, Object);

	public:
		struct ENGINE_EXPORT RenderData{};

		struct ENGINE_EXPORT LOD {
			RenderData render_data;
			Pointer<MaterialInterface> material;
		};

		Vector<LOD> lods;
	};
}// namespace Engine
