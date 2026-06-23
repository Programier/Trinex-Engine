#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/lifecycle.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/string_functions.hpp>
#include <Engine/project.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_cache.hpp>
#include <Graphics/shader_compiler.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	namespace
	{
		static String manifest_key(StringView object_path, Name permutation)
		{
			if (permutation == Name::none || !permutation.is_valid())
				return String(object_path);

			return Strings::format("{}::{}", object_path, permutation.to_string());
		}
	}// namespace

	static inline StringView find_rhi_name(const StringView& rhi_name)
	{
		if (rhi_name.empty())
		{
			return RHI::instance()->info.struct_instance->name().to_string();
		}
		return rhi_name;
	}

	static inline Path find_path(const StringView& object_path, const StringView& rhi_name)
	{
		return Strings::format("{}{}{}{}{}{}", Project::shader_cache_dir, Path::separator, rhi_name, Path::separator,
		                       Strings::replace_all(object_path, Constants::name_separator, Path::sv_separator),
		                       Constants::shader_extention);
	}

	static inline Path find_hash_path(u128 hash, StringView rhi_name)
	{
		const u64 high = static_cast<u64>(hash >> 64);
		const u64 low  = static_cast<u64>(hash);

		return Strings::format("[shader_cache]:/{}/{:016x}{:016x}{}", rhi_name, high, low, Constants::shader_extention);
	}

	static inline Path find_manifest_path(const StringView&, const StringView& rhi_name)
	{
		return Strings::format("{}{}{}{}Manifest{}", Project::shader_cache_dir, Path::separator, rhi_name, Path::separator,
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
			buffer = shader->source;
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
			shader->source = buffer;
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
		pipeline->vertex_attributes             = vertex_attributes;
		pipeline->vertex_shader(true)->source   = vertex;
		pipeline->fragment_shader(true)->source = fragment;
		pipeline->parameters(parameters);

		if (!tessellation_control.empty())
			pipeline->tessellation_control_shader(true)->source = tessellation_control;
		if (!tessellation.empty())
			pipeline->tessellation_shader(true)->source = tessellation;
		if (!geometry.empty())
			pipeline->geometry_shader(true)->source = geometry;
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

	void PipelineLibraryCache::init_from(const ShaderCompilationResult& compilation_result)
	{
		type = compilation_result.shaders.compute.empty() ? Graphics : Compute;

		switch (type)
		{
			case Graphics: graphics.init_from(compilation_result); break;
			case Compute: compute.init_from(compilation_result); break;
			default: break;
		}
	}

	void PipelineLibraryCache::apply_to(class Pipeline* pipeline)
	{
		switch (type)
		{
			case Graphics: graphics.apply_to(Object::instance_cast<GraphicsPipeline>(pipeline)); break;
			case Compute: compute.apply_to(Object::instance_cast<ComputePipeline>(pipeline)); break;
			default: break;
		}
	}

	bool PipelineLibraryCache::load_by_hash(u128 hash, StringView rhi_name)
	{
		rhi_name  = find_rhi_name(rhi_name);
		Path path = find_hash_path(hash, rhi_name);
		FileReader reader(path);

		if (!reader.is_open())
			return false;

		Archive ar(&reader);
		return serialize(ar);
	}

	bool PipelineLibraryCache::store_by_hash(u128 hash, StringView rhi_name) const
	{
		rhi_name  = find_rhi_name(rhi_name);
		Path path = find_hash_path(hash, rhi_name);
		rootfs()->create_dir(path.base_path());
		FileWriter writer(path);

		if (!writer.is_open())
		{
			error_log("PipelineLibraryCache", "Failed to open file '%s'", path.c_str());
			return false;
		}

		Archive ar(&writer);
		return const_cast<PipelineLibraryCache*>(this)->serialize(ar);
	}

	bool PipelineLibraryCache::serialize(Archive& ar)
	{
		u8 cache_type = static_cast<u8>(type);

		if (!ar.serialize(cache_type))
			return false;

		type = static_cast<Type>(cache_type);

		switch (type)
		{
			case Graphics: return graphics.serialize(ar);
			case Compute: return compute.serialize(ar);
			default: return false;
		}
	}

	const PipelineLibraryCacheIndexEntry* PipelineLibraryCacheManifest::find(StringView object_path, Name permutation) const
	{
		auto it = entries.find(manifest_key(object_path, permutation));
		return it == entries.end() ? nullptr : &it->second;
	}

	PipelineLibraryCacheIndexEntry& PipelineLibraryCacheManifest::entry(StringView object_path, Name permutation)
	{
		is_dirty = true;
		return entries[manifest_key(object_path, permutation)];
	}

	PipelineLibraryCacheManifest& PipelineLibraryCacheManifest::instance(StringView rhi)
	{
		static PipelineLibraryCacheManifest s_instance;

		if (!s_instance.is_loaded)
		{
			s_instance.load(rhi);
		}

		return s_instance;
	}

	bool PipelineLibraryCacheManifest::load(StringView new_rhi_name)
	{
		if (is_loaded)
			return true;

		rhi_name  = find_rhi_name(new_rhi_name);
		Path path = find_manifest_path("", rhi_name);
		entries.clear();
		FileReader reader(path);

		if (!reader.is_open())
		{
			is_loaded = true;
			return false;
		}

		Archive ar(&reader);
		is_loaded = serialize(ar);
		is_dirty  = false;
		return is_loaded;
	}

	bool PipelineLibraryCacheManifest::store() const
	{
		if (!is_loaded || !is_dirty)
			return true;

		Path path = find_manifest_path("", rhi_name);
		rootfs()->create_dir(path.base_path());
		FileWriter writer(path);

		if (!writer.is_open())
		{
			error_log("PipelineLibraryCacheManifest", "Failed to open file '%s'", path.c_str());
			return false;
		}

		Archive ar(&writer);
		const bool result = const_cast<PipelineLibraryCacheManifest*>(this)->serialize(ar);

		if (result)
		{
			const_cast<PipelineLibraryCacheManifest*>(this)->is_dirty = false;
		}

		return result;
	}

	bool PipelineLibraryCacheIndexEntry::serialize(Archive& ar)
	{
		u8 cache_type = static_cast<u8>(type);

		if (!ar.serialize(cache_type, shader_hash))
			return false;

		type = static_cast<PipelineLibraryCache::Type>(cache_type);
		return true;
	}

	bool PipelineLibraryCacheManifest::serialize(Archive& ar)
	{
		return ar.serialize(entries);
	}

	trinex_on_shutdown({.name = "PipelineLibraryCacheManifest"})
	{
		if (!PipelineLibraryCacheManifest::instance().store())
		{
			error_log("PipelineLibraryCacheManifest", "Failed to store pipeline library cache manifest");
		}
	}
}// namespace Trinex
