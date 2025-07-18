#include <Core/engine_loading_controllers.hpp>
#include <Graphics/editor_render_passes.hpp>
#include <Graphics/shader_compiler.hpp>

namespace Engine::EditorRenderPasses
{
	trinex_implement_render_pass(HitProxy);

	bool HitProxy::is_material_compatible(const Material* material)
	{
		return true;
	}

	RenderPass& HitProxy::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
	{
		env->add_module("editor/templates/hitproxy.slang");
		return *this;
	}
}// namespace Engine::EditorRenderPasses
