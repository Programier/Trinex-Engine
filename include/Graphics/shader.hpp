#pragma once
#include <Core/asset.hpp>
#include <Core/enums.hpp>
#include <Graphics/shader_parameters.hpp>
#include <RHI/resource_ptr.hpp>

namespace Trinex
{
	class RHIShader;
	class ENGINE_EXPORT Shader : public Asset
	{
		trinex_class(Shader, Asset);

	protected:
		RHIResourcePtr<RHIShader> m_shader;

	public:
		Buffer source;

		Shader& rebuild() override;
		inline RHIShader* handle() const { return m_shader; }
	};
}// namespace Trinex
