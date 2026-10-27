#include <Core/archive.hpp>
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
	trinex_implement_class(Trinex::StaticMesh, Refl::Class::IsAsset | Refl::Class::IsScriptable) {}

	trinex_implement_engine_class(SkeletalMesh, Refl::Class::IsAsset | Refl::Class::IsScriptable)
	{
		
	}

	bool MeshSurface::serialize(Archive& ar)
	{
		return ar.serialize(topology, first_vertex, first_index, vertices_count, material_index);
	}

	StaticMesh& StaticMesh::rebuild()
	{
		for (auto& lod : lods)
		{
			lod.vertex_stream.init();
			lod.surface_stream.init();
			lod.indices.init();
		}

		return *this;
	}

	template<typename Type>
	static void serialize_buffers(Archive& ar, Array<Type>& buffers)
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
		vertex_stream.serialize(ar);
		surface_stream.serialize(ar);
		indices.serialize(ar);
		return true;
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

	bool SkeletalMesh::LOD::serialize(Archive& ar)
	{
		vertex_stream.serialize(ar);
		surface_stream.serialize(ar);
		animation_stream.serialize(ar);
		indices.serialize(ar);
		return true;
	}

	SkeletalMesh& SkeletalMesh::rebuild()
	{
		for (auto& lod : lods)
		{
			lod.vertex_stream.init();
			lod.surface_stream.init();
			lod.animation_stream.init();
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

} // namespace Trinex
