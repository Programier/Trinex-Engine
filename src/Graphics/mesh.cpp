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

namespace Trinex
{
	trinex_implement_struct(Trinex::MeshSurface, 0)
	{
		trinex_refl_prop(topology, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
		trinex_refl_prop(first_vertex, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
		trinex_refl_prop(first_index, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
		trinex_refl_prop(vertices_count, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
		trinex_refl_prop(material_index, Refl::Property::IsTransient);
	}

	trinex_implement_struct(Trinex::StaticMesh::LOD, 0)
	{
		trinex_refl_prop(surfaces, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
	}

	trinex_implement_engine_class(StaticMesh, Refl::Class::IsAsset | Refl::Class::IsScriptable)
	{
		trinex_refl_prop(materials)->tooltip("Array of materials for this mesh");
		trinex_refl_prop(lods, Refl::Property::IsReadOnly | Refl::Property::IsTransient)->tooltip("Array of lods of this mesh");
	}

	trinex_implement_struct(Trinex::SkeletalMesh::LOD, 0)
	{
		trinex_refl_prop(surfaces, Refl::Property::IsReadOnly | Refl::Property::IsTransient);
	}

	trinex_implement_engine_class(SkeletalMesh, Refl::Class::IsAsset | Refl::Class::IsScriptable)
	{
		trinex_refl_prop(materials)->tooltip("Array of materials for this mesh");
		trinex_refl_prop(lods, Refl::Property::IsReadOnly | Refl::Property::IsTransient)->tooltip("Array of lods of this mesh");
	}

	bool MeshSurface::serialize(Archive& ar)
	{
		return ar.serialize(topology, first_vertex, first_index, vertices_count, material_index);
	}

	StaticMesh& StaticMesh::init_render_resources()
	{
		for (auto& lod : lods)
		{
			for (auto& buffer : lod.buffers) buffer.init();
			lod.indices.init();
		}

		return *this;
	}

	StaticMesh& StaticMesh::postload()
	{
		return init_render_resources();
	}

	template<typename Type>
	static void serialize_buffers(Archive& ar, Vector<Type>& buffers)
	{
		usize size = buffers.size();
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
		serialize_buffers(ar, buffers);
		indices.serialize(ar);
		return ar.serialize(surfaces, attributes);
	}

	bool StaticMesh::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;

		ar.serialize(bounds);

		usize lods_count = lods.size();
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

	VertexBufferBase* SkeletalMesh::LOD::find_position_buffer(usize index)
	{
		return positions.size() <= index ? nullptr : &positions[index];
	}

	VertexBufferBase* SkeletalMesh::LOD::find_tex_coord_buffer(usize index)
	{
		return tex_coords.size() <= index ? nullptr : &tex_coords[index];
	}

	VertexBufferBase* SkeletalMesh::LOD::find_color_buffer(usize index)
	{
		return colors.size() <= index ? nullptr : &colors[index];
	}

	VertexBufferBase* SkeletalMesh::LOD::find_normal_buffer(usize index)
	{
		return normals.size() <= index ? nullptr : &normals[index];
	}

	VertexBufferBase* SkeletalMesh::LOD::find_tangent_buffer(usize index)
	{
		return tangents.size() <= index ? nullptr : &tangents[index];
	}

	VertexBufferBase* SkeletalMesh::LOD::find_blend_weights_buffer(usize index)
	{
		return blend_weights.size() <= index ? nullptr : &blend_weights[index];
	}

	VertexBufferBase* SkeletalMesh::LOD::find_blend_indices_buffer(usize index)
	{
		return blend_indices.size() <= index ? nullptr : &blend_indices[index];
	}

	VertexBufferBase* SkeletalMesh::LOD::find_vertex_buffer(RHISemantic semantic, usize index)
	{
		using Func = VertexBufferBase* (*) (LOD * lod, usize);

		static Func funcs[12] = {
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_position_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_tex_coord_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_tex_coord_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_tex_coord_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_tex_coord_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_color_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_normal_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_tangent_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) -> VertexBufferBase* { return nullptr; }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_blend_weights_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) { return lod->find_blend_indices_buffer(index); }),
		        static_cast<Func>([](LOD* lod, usize index) -> VertexBufferBase* { return nullptr; }),
		};

		return funcs[semantic](this, index);
	}

	usize SkeletalMesh::LOD::vertex_count() const
	{
		for (auto& buffer : positions)
		{
			auto size = buffer.size();
			if (size > 0)
				return size;
		}
		return 0;
	}

	usize SkeletalMesh::LOD::indices_count() const
	{
		return indices.indices_count();
	}

	bool SkeletalMesh::LOD::serialize(Archive& ar)
	{
		serialize_buffers(ar, positions);
		serialize_buffers(ar, tex_coords);
		serialize_buffers(ar, colors);
		serialize_buffers(ar, normals);
		serialize_buffers(ar, tangents);
		serialize_buffers(ar, blend_weights);
		serialize_buffers(ar, blend_indices);
		indices.serialize(ar);
		return ar.serialize(surfaces);
	}

	SkeletalMesh& SkeletalMesh::init_render_resources()
	{
		for (auto& lod : lods)
		{
			for (auto& position : lod.positions) position.init();
			for (auto& coord : lod.tex_coords) coord.init();
			for (auto& color : lod.colors) color.init();
			for (auto& normal : lod.normals) normal.init();
			for (auto& tangent : lod.tangents) tangent.init();
			for (auto& blend_weights : lod.blend_weights) blend_weights.init();
			for (auto& blend_indices : lod.blend_indices) blend_indices.init();
			lod.indices.init();
		}

		return *this;
	}

	bool SkeletalMesh::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;

		ar.serialize(bounds);

		usize lods_count = lods.size();
		ar.serialize(lods_count);

		if (ar.is_reading())
		{
			lods.resize(lods_count);
		}

		for (auto& lod : lods)
		{
			lod.serialize(ar);
		}

		return ar.serialize(bones);
	}

	SkeletalMesh& SkeletalMesh::postload()
	{
		return init_render_resources();
	}
}// namespace Trinex
