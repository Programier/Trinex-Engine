#include <Core/archive.hpp>
#include <Core/file_manager.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/project.hpp>
#include <Graphics/shader_material.hpp>


namespace Engine
{
	implement_engine_class(ShaderMaterial, Refl::Class::IsAsset)
	{
		auto* self           = This::static_class_instance();
		auto change_callback = [](const Refl::PropertyChangedEvent& event) {
			auto self         = event.context_as<This>();
			self->shader_path = self->shader_path.relative(Project::shaders_dir);
		};

		trinex_refl_prop(self, This, shader_path)->push_change_listener(change_callback).tooltip("Path to slang file");
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
