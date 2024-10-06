#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/file_manager.hpp>
#include <Core/property.hpp>
#include <Engine/project.hpp>
#include <Graphics/shader_material.hpp>


namespace Engine
{
	implement_engine_class(ShaderMaterial, Class::IsAsset)
	{
		Class* self    = This::static_class_instance();
		auto path_prop = new ClassProperty("Shader Path", "Path to slang file", &This::shader_path, "Shader Material");

		path_prop->on_prop_changed.push([](void* object) {
			ShaderMaterial* material = reinterpret_cast<ShaderMaterial*>(object);
			material->shader_path    = material->shader_path.relative(Project::shaders_dir);
		});

		self->add_property(path_prop);
	}

	bool ShaderMaterial::shader_source(String& out_source)
	{
		FileReader reader(Path(Project::shaders_dir) / shader_path);
		if (reader.is_open())
		{
			out_source = reader.read_string();
			return true;
		}
		return false;
	}

	bool ShaderMaterial::archive_process(Archive& archive)
	{
		if (!Super::archive_process(archive))
			return false;
		return archive;
	}
}// namespace Engine
