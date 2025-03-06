#pragma once
#include <Core/filesystem/path.hpp>
#include <Graphics/material.hpp>

namespace Engine
{
	class ENGINE_EXPORT ShaderMaterial : public Material
	{
		trinex_declare_class(ShaderMaterial, Material);

	public:
		Path shader_path;

		bool shader_source(String& out_source) override;
		bool serialize(Archive& archive) override;
	};
}// namespace Engine
