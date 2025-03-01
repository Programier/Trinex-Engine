#pragma once
#include <Core/etl/map.hpp>
#include <Core/name.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	struct ENGINE_EXPORT ShaderCache {
		TreeMap<Name, MaterialParameterInfo> parameters;

		Buffer vertex;
		Buffer tessellation_control;
		Buffer tessellation;
		Buffer geometry;
		Buffer fragment;
		Buffer compute;

		MaterialScalarParametersInfo local_parameters;

		void init_from(const class GraphicsPipeline* pipeline);
		void apply_to(class GraphicsPipeline* pipeline);
		bool load(const StringView& object_path, StringView rhi_name = {});
		bool store(const StringView& object_path, StringView rhi_name = {}) const;
		bool serialize(Archive& ar);
	};
}// namespace Engine
