#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Core/name.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	struct ShaderCompilationResult;
	
	struct ENGINE_EXPORT GraphicsShaderCache {
		Vector<RHIShaderParameterInfo> parameters;
		Vector<RHIVertexAttribute> vertex_attributes;
		Buffer vertex;
		Buffer tessellation_control;
		Buffer tessellation;
		Buffer geometry;
		Buffer fragment;

		void init_from(const class GraphicsPipeline* pipeline);
		void init_from(const ShaderCompilationResult& compilation_result);
		void apply_to(class GraphicsPipeline* pipeline);
		bool load(const StringView& object_path, StringView rhi_name = {});
		bool store(const StringView& object_path, StringView rhi_name = {}) const;
		bool serialize(Archive& ar);
	};

	struct ENGINE_EXPORT ComputeShaderCache {
		Vector<RHIShaderParameterInfo> parameters;

		Buffer compute;

		void init_from(const class ComputePipeline* pipeline);
		void init_from(const ShaderCompilationResult& compilation_result);
		void apply_to(class ComputePipeline* pipeline);
		bool load(const StringView& object_path, StringView rhi_name = {});
		bool store(const StringView& object_path, StringView rhi_name = {}) const;
		bool serialize(Archive& ar);
	};
}// namespace Engine
