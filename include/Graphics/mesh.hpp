#pragma once
#include <Core/enums.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
	class MaterialInterface;
	class VertexBuffer;
	class UInt32IndexBuffer;

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
			Vector<Pointer<class PositionVertexBuffer>> positions;
			Vector<Pointer<class TexCoordVertexBuffer>> tex_coords;
			Vector<Pointer<class ColorVertexBuffer>> colors;
			Vector<Pointer<class NormalVertexBuffer>> normals;
			Vector<Pointer<class TangentVertexBuffer>> tangents;
			Vector<Pointer<class BitangentVertexBuffer>> bitangents;
			Pointer<UInt32IndexBuffer> indices;

			Vector<MeshSurface> surfaces;

		private:
			VertexBuffer* find_position_buffer(Index index) const;
			VertexBuffer* find_tex_coord_buffer(Index index) const;
			VertexBuffer* find_color_buffer(Index index) const;
			VertexBuffer* find_normal_buffer(Index index) const;
			VertexBuffer* find_tangent_buffer(Index index) const;
			VertexBuffer* find_bitangent_buffer(Index index) const;

		public:
			VertexBuffer* find_vertex_buffer(VertexBufferSemantic semantic, Index index = 0) const;
			size_t vertex_count() const;
			size_t indices_count() const;
			bool serialize(Archive& ar, bool allow_cpu_access = false);
		};

		Vector<MeshMaterial> materials;
		AABB_3Df bounds;
		Vector<LOD> lods;
		bool allow_cpu_access = false;

		StaticMesh();
		StaticMesh& init_resources();
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
