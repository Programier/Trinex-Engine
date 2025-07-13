#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/string_functions.hpp>
#include <Engine/project.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_cache.hpp>
#include <Graphics/shader_compiler.hpp>
#include <RHI/rhi.hpp>

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

	template<typename T>
	static bool load_shader_cache(T* cache, const StringView& object_path, StringView rhi_name, const char* tag)
	{
		rhi_name  = find_rhi_name(rhi_name);
		Path path = find_path(object_path, rhi_name);
		FileReader reader(path);

		if (!reader.is_open())
		{
			error_log(tag, "Failed to open file '%s'", path.c_str());
			return false;
		}

		Archive ar(&reader);
		return cache->serialize(ar);
	}

	template<typename T>
	static bool store_shader_cache(const T* cache, const StringView& object_path, StringView rhi_name, const char* tag)
	{
		rhi_name  = find_rhi_name(rhi_name);
		Path path = find_path(object_path, rhi_name);
		rootfs()->create_dir(path.base_path());
		FileWriter writer(path);

		if (!writer.is_open())
		{
			error_log(tag, "Failed to open file '%s'", path.c_str());
			return false;
		}

		Archive ar(&writer);
		return const_cast<T*>(cache)->serialize(ar);
	}

	bool GraphicsShaderCache::serialize(Archive& ar)
	{
		return ar.serialize(parameters, vertex_attributes, vertex, tessellation_control, tessellation, geometry, fragment);
	}

	bool GraphicsShaderCache::load(const StringView& object_path, StringView rhi_name)
	{
		return load_shader_cache(this, object_path, rhi_name, "GraphicsShaderCache");
	}

	bool GraphicsShaderCache::store(const StringView& object_path, StringView rhi_name) const
	{
		return store_shader_cache(this, object_path, rhi_name, "GraphicsShaderCache");
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

	void GraphicsShaderCache::init_from(const class GraphicsPipeline* pipeline)
	{
		parameters        = pipeline->parameters();
		vertex_attributes = pipeline->vertex_attributes;

		copy_buffer(vertex, pipeline->vertex_shader());
		copy_buffer(tessellation_control, pipeline->tessellation_control_shader());
		copy_buffer(tessellation, pipeline->tessellation_shader());
		copy_buffer(geometry, pipeline->geometry_shader());
		copy_buffer(fragment, pipeline->fragment_shader());
	}

	void GraphicsShaderCache::init_from(const ShaderCompilationResult& compilation_result)
	{
		parameters        = compilation_result.reflection.parameters;
		vertex_attributes = compilation_result.reflection.vertex_attributes;

		vertex               = compilation_result.shaders.vertex;
		tessellation_control = compilation_result.shaders.tessellation_control;
		tessellation         = compilation_result.shaders.tessellation;
		geometry             = compilation_result.shaders.geometry;
		fragment             = compilation_result.shaders.fragment;
	}

	void GraphicsShaderCache::apply_to(class GraphicsPipeline* pipeline)
	{
		if (vertex.empty() || fragment.empty())
			return;

		pipeline->clear();
		pipeline->vertex_attributes                  = vertex_attributes;
		pipeline->vertex_shader(true)->source_code   = vertex;
		pipeline->fragment_shader(true)->source_code = fragment;
		pipeline->parameters(parameters);

		if (!tessellation_control.empty())
			pipeline->tessellation_control_shader(true)->source_code = tessellation_control;
		if (!tessellation.empty())
			pipeline->tessellation_shader(true)->source_code = tessellation;
		if (!geometry.empty())
			pipeline->geometry_shader(true)->source_code = geometry;
	}

	void ComputeShaderCache::init_from(const class ComputePipeline* pipeline)
	{
		parameters = pipeline->parameters();
		copy_buffer(compute, pipeline->compute_shader());
	}

	void ComputeShaderCache::init_from(const ShaderCompilationResult& compilation_result)
	{
		parameters = compilation_result.reflection.parameters;
		compute    = compilation_result.shaders.compute;
	}

	void ComputeShaderCache::apply_to(class ComputePipeline* pipeline)
	{
		pipeline->parameters(parameters);
		apply_buffer(compute, pipeline->compute_shader());
	}

	bool ComputeShaderCache::load(const StringView& object_path, StringView rhi_name)
	{
		return load_shader_cache(this, object_path, rhi_name, "ComputeShaderCache");
	}

	bool ComputeShaderCache::store(const StringView& object_path, StringView rhi_name) const
	{
		return store_shader_cache(this, object_path, rhi_name, "ComputeShaderCache");
	}

	bool ComputeShaderCache::serialize(Archive& ar)
	{
		return ar.serialize(parameters, compute);
	}
}// namespace Engine
