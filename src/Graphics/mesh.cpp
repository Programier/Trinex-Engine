#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/property.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/struct.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	implement_struct(Engine::MeshMaterial)
	{
		auto* self = static_struct_instance();
		self->add_properties(new ClassProperty("Surface Index", "Surface Index", &MeshMaterial::surface_index),
		                     new ObjectReferenceProperty("Material", "Material which used for rendering this primitive",
		                                                 &MeshMaterial::material));
	}

	implement_engine_class(StaticMesh, Refl::Class::IsAsset)
	{
		auto* self                  = StaticMesh::static_class_instance();
		auto* mesh_materials_struct = Refl::Struct::static_find("Engine::MeshMaterial", Refl::FindFlags::IsRequired);
		auto mesh_material_prop     = new StructProperty<This, MeshMaterial>("", "", nullptr, mesh_materials_struct);
		self->add_property(new ArrayProperty<This, decltype(materials)>("Materials", "Array of materials for this primitive",
		                                                                &This::materials, mesh_material_prop));
	}

	implement_engine_class_default_init(DynamicMesh, 0);

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

	VertexBuffer* StaticMesh::LOD::find_binormal_buffer(Index index) const
	{
		return binormals.size() <= index ? nullptr : binormals[index].ptr();
	}


	StaticMesh::StaticMesh()
	{
		materials.resize(1);
		auto& entry    = materials.back();
		entry.policy   = 0;
		entry.material = Object::static_find_object_checked<MaterialInterface>("DefaultPackage::DefaultMaterial");
	}

	StaticMesh& StaticMesh::init_resources()
	{
		for (auto& lod : lods)
		{
			for (auto& position : lod.positions)
			{
				if (position)
					position->init_resource();
			}

			for (auto& coord : lod.tex_coords)
			{
				if (coord)
					coord->init_resource();
			}

			for (auto& color : lod.colors)
			{
				if (color)
					color->init_resource();
			}

			for (auto& normal : lod.normals)
			{
				if (normal)
					normal->init_resource();
			}

			for (auto& tangent : lod.tangents)
			{
				if (tangent)
					tangent->init_resource();
			}

			for (auto& binormal : lod.binormals)
			{
				if (binormal)
					binormal->init_resource();
			}

			if (lod.indices)
			{
				lod.indices->init_resource();
			}
		}

		return *this;
	}

	StaticMesh& StaticMesh::apply_changes()
	{
		return init_resources();
	}

	StaticMesh& StaticMesh::postload()
	{
		return init_resources();
	}


	template<typename Type>
	static void serialize_buffer(Archive& ar, Pointer<Type>& buffer)
	{
		bool is_valid = buffer;
		ar & is_valid;

		if (is_valid)
		{
			if (ar.is_reading())
			{
				buffer = Object::new_instance<Type>();
			}

			buffer.ptr()->archive_process(ar);
		}
	}

	template<typename Type>
	static void serialize_buffers(Archive& ar, Vector<Pointer<Type>>& buffers)
	{
		size_t size = buffers.size();
		ar & size;

		if (size > 0)
		{
			if (ar.is_reading())
			{
				buffers.resize(size);
			}

			for (auto& buffer : buffers)
			{
				serialize_buffer(ar, buffer);
			}
		}
	}

	ENGINE_EXPORT bool operator&(Archive& ar, MeshSurface& surface)
	{
		ar & surface.base_vertex_index;
		ar & surface.first_index;
		ar & surface.vertices_count;
		return ar;
	}

	ENGINE_EXPORT bool operator&(Archive& ar, StaticMesh::LOD& lod)
	{
		serialize_buffers(ar, lod.positions);
		serialize_buffers(ar, lod.tex_coords);
		serialize_buffers(ar, lod.colors);
		serialize_buffers(ar, lod.normals);
		serialize_buffers(ar, lod.tangents);
		serialize_buffers(ar, lod.binormals);
		serialize_buffer(ar, lod.indices);
		ar & lod.surfaces;
		return ar;
	}


	bool StaticMesh::archive_process(Archive& ar)
	{
		if (!Super::archive_process(ar))
			return false;

		ar & bounds;
		ar & lods;

		return ar;
	}

	VertexBuffer* StaticMesh::LOD::find_vertex_buffer(VertexBufferSemantic semantic, Index index) const
	{
		Index semantic_index = static_cast<Index>(semantic);
		if (semantic_index > static_cast<Index>(VertexBufferSemantic::Binormal))
		{
			return nullptr;
		}

		static VertexBuffer* (StaticMesh::LOD::*find_buffer_private[])(Index) const = {
		        &StaticMesh::LOD::find_position_buffer, &StaticMesh::LOD::find_tex_coord_buffer,
		        &StaticMesh::LOD::find_color_buffer,    &StaticMesh::LOD::find_normal_buffer,
		        &StaticMesh::LOD::find_tangent_buffer,  &StaticMesh::LOD::find_binormal_buffer,
		};

		return ((*this).*(find_buffer_private[semantic_index]))(index);
	}

	size_t StaticMesh::LOD::vertex_count() const
	{
		for (auto& buffer : positions)
		{
			if (buffer.ptr())
			{
				return buffer->elements_count();
			}
		}
		return 0;
	}

	size_t StaticMesh::LOD::indices_count() const
	{
		return indices.ptr() ? indices->elements_count() : 0;
	}
}// namespace Engine
