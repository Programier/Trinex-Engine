#include <Core/archive.hpp>
#include <Core/blob.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/project.hpp>
#include <Graphics/shader_material.hpp>


namespace Trinex
{
	trinex_implement_engine_class(ShaderMaterial, Refl::Class::IsAsset)
	{
		trinex_refl_prop(shader_path)->tooltip("Path to slang file");
	}

	bool ShaderMaterial::shader_source(String& out_source)
	{
		Path path = shader_path;

		if (auto buffer = rootfs()->map(path))
		{
			out_source = buffer->as<StringView>();
			return true;
		}
		return false;
	}

	bool ShaderMaterial::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return archive;
	}
}// namespace Trinex
