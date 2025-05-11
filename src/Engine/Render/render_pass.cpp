#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/map.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Graphics/shader_compiler.hpp>

namespace Engine
{
	static Map<Name, RenderPass*, Name::HashFunction> s_render_pass_table;

	RenderPass* RenderPass::s_head = nullptr;
	RenderPass* RenderPass::s_tail = nullptr;

	RenderPass::RenderPass(const char* name) : m_name(name)
	{
		s_render_pass_table.insert({name, this});

		if (s_head == nullptr)
		{
			s_head = s_tail = this;
		}
		else
		{
			s_tail->m_next = this;
			s_tail         = this;
		}
	}

	RenderPass* RenderPass::static_find(const Name& name)
	{
		auto it = s_render_pass_table.find(name);

		if (it == s_render_pass_table.end())
			return nullptr;

		return it->second;
	}

	bool RenderPass::is_material_compatible(const Material* material)
	{
		return true;
	}

	RenderPass& RenderPass::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
	{
		return *this;
	}

	RenderPass* RenderPass::super_pass()
	{
		return nullptr;
	}

	RenderPass::~RenderPass()
	{
		s_render_pass_table.erase(name());
	}

	namespace RenderPasses
	{
		trinex_implement_render_pass(Depth);
		trinex_implement_render_pass(GenericOutput);
		trinex_implement_render_pass(GenericGeometry);


		GenericGeometry& GenericGeometry::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
		{
			Super::modify_shader_compilation_env(env);
			env->add_definition_nocopy("TRINEX_GEOMETRY_PASS", "1");
			return *this;
		}
	}// namespace RenderPasses
}// namespace Engine
