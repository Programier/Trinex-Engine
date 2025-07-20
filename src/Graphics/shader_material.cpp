#include <Core/archive.hpp>
#include <Core/file_manager.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/project.hpp>
#include <Graphics/shader_material.hpp>


namespace Engine
{
	trinex_implement_engine_class(ShaderMaterial, Refl::Class::IsAsset)
	{
		trinex_refl_prop(shader_path)->tooltip("Path to slang file");
	}

	bool ShaderMaterial::shader_source(String& out_source)
	{
		FileReader reader(shader_path);
		if (reader.is_open())
		{
			out_source = reader.read_string();
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
}// namespace Engine
