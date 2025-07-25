#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	trinex_implement_struct(Engine::MeshSurface, 0)
	{
		trinex_refl_prop(base_vertex_index, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
		trinex_refl_prop(first_index, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
		trinex_refl_prop(vertices_count, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
		trinex_refl_prop(material_index, Refl::Property::IsTransient);
	}

	trinex_implement_struct(Engine::StaticMesh::LOD, 0)
	{
		trinex_refl_prop(surfaces, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
	}

	trinex_implement_engine_class(StaticMesh, Refl::Class::IsAsset | Refl::Class::IsScriptable)
	{
		trinex_refl_prop(materials)->tooltip("Array of materials for this mesh");
		trinex_refl_prop(lods, Refl::Property::IsReadOnly | Refl::Property::IsTransient)->tooltip("Array of lods of this mesh");
	}

	trinex_implement_engine_class_default_init(SkeletalMesh, 0);

	bool MeshSurface::serialize(Archive& ar)
	{
		return ar.serialize(base_vertex_index, first_index, vertices_count, material_index);
	}

	PositionVertexBuffer* StaticMesh::LOD::find_position_buffer(Index index)
	{
		return positions.size() <= index ? nullptr : &positions[index];
	}

	TexCoordVertexBuffer* StaticMesh::LOD::find_tex_coord_buffer(Index index)
	{
		return tex_coords.size() <= index ? nullptr : &tex_coords[index];
	}

	ColorVertexBuffer* StaticMesh::LOD::find_color_buffer(Index index)
	{
		return colors.size() <= index ? nullptr : &colors[index];
	}

	NormalVertexBuffer* StaticMesh::LOD::find_normal_buffer(Index index)
	{
		return normals.size() <= index ? nullptr : &normals[index];
	}

	TangentVertexBuffer* StaticMesh::LOD::find_tangent_buffer(Index index)
	{
		return tangents.size() <= index ? nullptr : &tangents[index];
	}

	BitangentVertexBuffer* StaticMesh::LOD::find_bitangent_buffer(Index index)
	{
		return bitangents.size() <= index ? nullptr : &bitangents[index];
	}

	StaticMesh::StaticMesh() : materials({DefaultResources::Materials::base_pass}) {}

	StaticMesh& StaticMesh::init_render_resources()
	{
		for (auto& lod : lods)
		{
			for (auto& position : lod.positions) position.init();
			for (auto& coord : lod.tex_coords) coord.init();
			for (auto& color : lod.colors) color.init();
			for (auto& normal : lod.normals) normal.init();
			for (auto& tangent : lod.tangents) tangent.init();
			for (auto& bitangent : lod.bitangents) bitangent.init();
			lod.indices.init();
		}

		return *this;
	}

	StaticMesh& StaticMesh::apply_changes()
	{
		return init_render_resources();
	}

	StaticMesh& StaticMesh::postload()
	{
		return init_render_resources();
	}

	template<typename Type>
	static void serialize_buffers(Archive& ar, Vector<Type>& buffers)
	{
		size_t size = buffers.size();
		ar.serialize(size);

		if (size > 0)
		{
			if (ar.is_reading())
			{
				buffers.resize(size);
			}

			for (auto& buffer : buffers)
			{
				buffer.serialize(ar);
			}
		}
	}

	bool StaticMesh::LOD::serialize(Archive& ar)
	{
		serialize_buffers(ar, positions);
		serialize_buffers(ar, tex_coords);
		serialize_buffers(ar, colors);
		serialize_buffers(ar, normals);
		serialize_buffers(ar, tangents);
		serialize_buffers(ar, bitangents);
		indices.serialize(ar);
		return ar.serialize(surfaces);
	}

	bool StaticMesh::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;

		ar.serialize(bounds);

		size_t lods_count = lods.size();
		ar.serialize(lods_count);

		if (ar.is_reading())
		{
			lods.resize(lods_count);
		}

		for (auto& lod : lods)
		{
			lod.serialize(ar);
		}
		return ar;
	}

	VertexBufferBase* StaticMesh::LOD::find_vertex_buffer(RHIVertexBufferSemantic semantic, Index index)
	{
		Index semantic_index = static_cast<Index>(semantic);
		if (semantic_index > static_cast<Index>(RHIVertexBufferSemantic::Bitangent))
		{
			return nullptr;
		}

		using Func = VertexBufferBase* (*) (LOD * lod, size_t);

		static Func funcs[6] = {
		        static_cast<Func>([](LOD* lod, size_t index) -> VertexBufferBase* { return lod->find_position_buffer(index); }),
		        static_cast<Func>([](LOD* lod, size_t index) -> VertexBufferBase* { return lod->find_tex_coord_buffer(index); }),
		        static_cast<Func>([](LOD* lod, size_t index) -> VertexBufferBase* { return lod->find_color_buffer(index); }),
		        static_cast<Func>([](LOD* lod, size_t index) -> VertexBufferBase* { return lod->find_normal_buffer(index); }),
		        static_cast<Func>([](LOD* lod, size_t index) -> VertexBufferBase* { return lod->find_tangent_buffer(index); }),
		        static_cast<Func>([](LOD* lod, size_t index) -> VertexBufferBase* { return lod->find_bitangent_buffer(index); }),
		};

		return funcs[semantic](this, index);
	}

	size_t StaticMesh::LOD::vertex_count() const
	{
		for (auto& buffer : positions)
		{
			auto size = buffer.size();
			if (size > 0)
				return size;
		}
		return 0;
	}

	size_t StaticMesh::LOD::indices_count() const
	{
		return indices.indices_count();
	}
}// namespace Engine
