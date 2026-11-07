#pragma once
#include <Core/asset.hpp>
#include <Core/enums.hpp>
#include <Graphics/shader_parameters.hpp>

namespace Trinex
{
	class RHIShader;
	class trinex_api Shader : public Asset
	{
		trinex_class(Shader, Asset);

	protected:
		Ref<RHIShader> m_shader;

	public:
		Buffer source;

		Shader& rebuild() override;
		inline RHIShader* handle() const { return m_shader; }
	};
} // namespace Trinex
