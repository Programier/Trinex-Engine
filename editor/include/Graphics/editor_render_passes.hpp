#pragma once
#include <Engine/Render/render_pass.hpp>

namespace Engine::EditorRenderPasses
{
	class HitProxy : public RenderPass
	{
		trinex_render_pass(HitProxy, RenderPass);

	public:
		bool is_material_compatible(const Material* material) override;
		RenderPass& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
	};
}// namespace Engine::EditorRenderPasses
