#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/string_functions.hpp>
#include <Engine/project.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_cache.hpp>

namespace Engine
{

	static inline StringView find_rhi_name(const StringView& rhi_name)
	{
		if (rhi_name.empty())
		{
			return rhi->info.struct_instance->name().to_string();
		}
		return rhi_name;
	}

	static inline Path find_path(const StringView& object_path, const StringView& rhi_name)
	{
		return Strings::format("{}{}{}{}{}{}", Project::shader_cache_dir, Path::separator, rhi_name, Path::separator,
		                       Strings::replace_all(object_path, Constants::name_separator, Path::sv_separator),
		                       Constants::shader_extention);
	}

	bool ShaderCache::serialize(Archive& ar)
	{
		return ar.serialize(parameters, vertex, tessellation_control, tessellation, geometry, fragment, compute,
							local_parameters);
	}

	bool ShaderCache::load(const StringView& object_path, StringView rhi_name)
	{
		rhi_name  = find_rhi_name(rhi_name);
		Path path = find_path(object_path, rhi_name);
		FileReader reader(path);

		if (!reader.is_open())
		{
			error_log("ShaderCache", "Failed to open file '%s'", path.c_str());
			return false;
		}

		Archive ar(&reader);
		return serialize(ar);
	}

	bool ShaderCache::store(const StringView& object_path, StringView rhi_name) const
	{
		rhi_name  = find_rhi_name(rhi_name);
		Path path = find_path(object_path, rhi_name);
		rootfs()->create_dir(path.base_path());
		FileWriter writer(path);

		if (!writer.is_open())
		{
			error_log("ShaderCache", "Failed to open file '%s'", path.c_str());
			return false;
		}

		Archive ar(&writer);
		return const_cast<ShaderCache*>(this)->serialize(ar);
	}

	static inline void copy_buffer(Buffer& buffer, Shader* shader)
	{
		if (shader)
		{
			buffer = shader->source_code;
		}
		else
		{
			buffer.clear();
			buffer.shrink_to_fit();
		}
	}

	static inline void apply_buffer(const Buffer& buffer, Shader* shader)
	{
		if (shader)
		{
			shader->source_code = buffer;
		}
	}

	void ShaderCache::init_from(const class Pipeline* pipeline)
	{
		parameters = pipeline->parameters;

		copy_buffer(vertex, pipeline->vertex_shader());
		copy_buffer(tessellation_control, pipeline->tessellation_control_shader());
		copy_buffer(tessellation, pipeline->tessellation_shader());
		copy_buffer(geometry, pipeline->geometry_shader());
		copy_buffer(fragment, pipeline->fragment_shader());
		copy_buffer(compute, nullptr);
	}

	void ShaderCache::apply_to(class Pipeline* pipeline)
	{
		pipeline->parameters = parameters;

		apply_buffer(vertex, pipeline->vertex_shader());
		apply_buffer(tessellation_control, pipeline->tessellation_control_shader());
		apply_buffer(tessellation, pipeline->tessellation_shader());
		apply_buffer(geometry, pipeline->geometry_shader());
		apply_buffer(fragment, pipeline->fragment_shader());
		apply_buffer(compute, nullptr);
	}
}// namespace Engine
