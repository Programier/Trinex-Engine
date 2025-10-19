#pragma once
#include <Engine/Render/render_pass.hpp>

namespace Engine::EditorRenderPasses
{
	class HitProxy : public RenderPass
	{
		trinex_render_pass(HitProxy, RenderPass);

	public:
		HitProxy& apply_blending_state(RHIContext* ctx, const RHIBlendingState& state) override;
		
		bool is_material_compatible(const Material* material) override;
		RenderPass& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
	};
}// namespace Engine::EditorRenderPasses
