#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	trinex_implement_struct(Engine::MeshMaterial, 0)
	{
		auto* self = static_struct_instance();

		trinex_refl_prop(self, This, surface_index);
		trinex_refl_prop(self, This, material)->tooltip("Material which used for rendering this primitive");
	}

	trinex_implement_engine_class(StaticMesh, Refl::Class::IsAsset | Refl::Class::IsScriptable)
	{
		auto* self = StaticMesh::static_class_instance();
		trinex_refl_prop(self, This, materials)->tooltip("Array of materials for this primitive");
		trinex_refl_prop(self, This, allow_cpu_access);
	}

	trinex_implement_engine_class_default_init(DynamicMesh, 0);

	VertexBuffer* StaticMesh::LOD::find_position_buffer(Index index) const
	{
		return positions.size() <= index ? nullptr : positions[index].ptr();
	}

	VertexBuffer* StaticMesh::LOD::find_tex_coord_buffer(Index index) const
	{
		return tex_coords.size() <= index ? nullptr : tex_coords[index].ptr();
	}

	VertexBuffer* StaticMesh::LOD::find_color_buffer(Index index) const
	{
		return colors.size() <= index ? nullptr : colors[index].ptr();
	}

	VertexBuffer* StaticMesh::LOD::find_normal_buffer(Index index) const
	{
		return normals.size() <= index ? nullptr : normals[index].ptr();
	}

	VertexBuffer* StaticMesh::LOD::find_tangent_buffer(Index index) const
	{
		return tangents.size() <= index ? nullptr : tangents[index].ptr();
	}

	VertexBuffer* StaticMesh::LOD::find_bitangent_buffer(Index index) const
	{
		return bitangents.size() <= index ? nullptr : bitangents[index].ptr();
	}


	StaticMesh::StaticMesh()
	{
		materials.resize(1);
		auto& entry    = materials.back();
		entry.policy   = 0;
		entry.material = Object::static_find_object_checked<MaterialInterface>("DefaultPackage::DefaultMaterial");
	}

	StaticMesh& StaticMesh::init_render_resources()
	{
		for (auto& lod : lods)
		{
			for (auto& position : lod.positions)
			{
				if (position)
					position->init_render_resources();
			}

			for (auto& coord : lod.tex_coords)
			{
				if (coord)
					coord->init_render_resources();
			}

			for (auto& color : lod.colors)
			{
				if (color)
					color->init_render_resources();
			}

			for (auto& normal : lod.normals)
			{
				if (normal)
					normal->init_render_resources();
			}

			for (auto& tangent : lod.tangents)
			{
				if (tangent)
					tangent->init_render_resources();
			}

			for (auto& bitangent : lod.bitangents)
			{
				if (bitangent)
					bitangent->init_render_resources();
			}

			if (lod.indices)
			{
				lod.indices->init_render_resources();
			}
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
	static void serialize_buffer(Archive& ar, Pointer<Type>& buffer, bool allow_cpu_access)
	{
		bool is_valid = buffer;
		ar.serialize(is_valid);

		if (is_valid)
		{
			if (ar.is_reading())
			{
				buffer = Object::new_instance<Type>();
			}

			buffer.ptr()->serialize(ar);
		}
	}

	template<typename Type>
	static void serialize_buffers(Archive& ar, Vector<Pointer<Type>>& buffers, bool allow_cpu_access)
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
				serialize_buffer(ar, buffer, allow_cpu_access);
			}
		}
	}

	bool StaticMesh::LOD::serialize(Archive& ar, bool allow_cpu_access)
	{
		serialize_buffers(ar, positions, allow_cpu_access);
		serialize_buffers(ar, tex_coords, allow_cpu_access);
		serialize_buffers(ar, colors, allow_cpu_access);
		serialize_buffers(ar, normals, allow_cpu_access);
		serialize_buffers(ar, tangents, allow_cpu_access);
		serialize_buffers(ar, bitangents, allow_cpu_access);
		serialize_buffer(ar, indices, allow_cpu_access);
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
			lod.serialize(ar, allow_cpu_access);
		}
		return ar;
	}

	VertexBuffer* StaticMesh::LOD::find_vertex_buffer(VertexBufferSemantic semantic, Index index) const
	{
		Index semantic_index = static_cast<Index>(semantic);
		if (semantic_index > static_cast<Index>(VertexBufferSemantic::Bitangent))
		{
			return nullptr;
		}

		static VertexBuffer* (StaticMesh::LOD::* find_buffer_private[])(Index) const = {
		        &StaticMesh::LOD::find_position_buffer, &StaticMesh::LOD::find_tex_coord_buffer,
		        &StaticMesh::LOD::find_color_buffer,    &StaticMesh::LOD::find_normal_buffer,
				&StaticMesh::LOD::find_tangent_buffer,  &StaticMesh::LOD::find_bitangent_buffer,
		};

		return ((*this).*(find_buffer_private[semantic_index]))(index);
	}

	size_t StaticMesh::LOD::vertex_count() const
	{
		for (auto& buffer : positions)
		{
			if (buffer.ptr())
			{
				return buffer->vertex_count();
			}
		}
		return 0;
	}

	size_t StaticMesh::LOD::indices_count() const
	{
		return indices.ptr() ? indices->index_count() : 0;
	}
}// namespace Engine
