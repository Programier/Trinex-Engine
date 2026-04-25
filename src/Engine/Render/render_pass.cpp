#include <Core/etl/map.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/shader_compiler.hpp>
#include <RHI/context.hpp>

namespace Trinex
{
	static Map<Name, RenderPass*, Name::HashFunction> s_render_pass_table;

	RenderPass* RenderPass::s_head = nullptr;
	RenderPass* RenderPass::s_tail = nullptr;

	RenderPass::RenderPass(const char* name, RenderPass* owner) : m_name(name)
	{
		m_next                = owner->m_permutations;
		owner->m_permutations = static_cast<RenderPassPermutation*>(this);
	}

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

	RenderPass& RenderPass::begin(Renderer* renderer, RHIContext* ctx)
	{
		return *this;
	}

	bool RenderPass::depth_stencil_state(RHIDepthStencilState& state)
	{
		state.depth.func = state.depth.is_enabled() ? RHICompareFunc::Gequal : RHICompareFunc::Always;
		return true;
	}

	bool RenderPass::blending_state(RHIBlendingState& state)
	{
		return true;
	}

	bool RenderPass::rasterizer_state(RHIRasterizerState& state)
	{
		return true;
	}

	RenderPass& RenderPass::end(Renderer* renderer, RHIContext* ctx)
	{
		return *this;
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

	String RenderPass::full_name() const
	{
		return m_name.to_string();
	}

	RenderPassPermutation* RenderPass::find_permutation(const Name& name) const
	{
		RenderPassPermutation* entry = m_permutations;

		while (entry && entry->name() != name)
		{
			entry = static_cast<RenderPassPermutation*>(entry->next());
		}

		return entry;
	}

	RenderPass::~RenderPass()
	{
		s_render_pass_table.erase(name());
	}

	RenderPassPermutation::RenderPassPermutation(const char* name, RenderPass* owner) : RenderPass(name, owner), m_owner(owner) {}

	RenderPassPermutation& RenderPassPermutation::begin(Renderer* renderer, RHIContext* ctx)
	{
		m_owner->begin(renderer, ctx);
		return *this;
	}

	bool RenderPassPermutation::depth_stencil_state(RHIDepthStencilState& state)
	{
		return m_owner->depth_stencil_state(state);
	}

	bool RenderPassPermutation::blending_state(RHIBlendingState& state)
	{
		return m_owner->blending_state(state);
	}

	bool RenderPassPermutation::rasterizer_state(RHIRasterizerState& state)
	{
		return m_owner->rasterizer_state(state);
	}

	RenderPassPermutation& RenderPassPermutation::end(Renderer* renderer, RHIContext* ctx)
	{
		m_owner->end(renderer, ctx);
		return *this;
	}

	bool RenderPassPermutation::is_material_compatible(const Material* material)
	{
		return m_owner->is_material_compatible(material);
	}

	RenderPassPermutation& RenderPassPermutation::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
	{
		m_owner->modify_shader_compilation_env(env);
		return *this;
	}

	String RenderPassPermutation::full_name() const
	{
		return name().to_string() + m_owner->full_name();
	}

	namespace RenderPassPermutations
	{
		StaticMesh& StaticMesh::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
		{
			RenderPassPermutation::modify_shader_compilation_env(env);
			env->add_module("trinex/vertex_inputs/static_mesh.slang");
			return *this;
		}

		SkeletalMesh& SkeletalMesh::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
		{
			RenderPassPermutation::modify_shader_compilation_env(env);
			env->add_module("trinex/vertex_inputs/skeletal_mesh.slang");
			return *this;
		}
	}// namespace RenderPassPermutations

	namespace RenderPasses
	{
		trinex_implement_render_pass(Depth)
		{
			static RenderPassPermutations::StaticMesh static_mesh(this);
			static RenderPassPermutations::SkeletalMesh skeletal_mesh(this);
		}

		trinex_implement_render_pass(Geometry)
		{
			static RenderPassPermutations::StaticMesh static_mesh(this);
			static RenderPassPermutations::SkeletalMesh skeletal_mesh(this);
		}

		trinex_implement_render_pass(Translucent)
		{
			static RenderPassPermutations::StaticMesh static_mesh(this);
			static RenderPassPermutations::SkeletalMesh skeletal_mesh(this);
		}

		bool Depth::is_material_compatible(const Material* material)
		{
			return material->domain == MaterialDomain::Surface && material->blend_mode.is_opaque();
		}

		Depth& Depth::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
		{
			Super::modify_shader_compilation_env(env);
			env->add_module("trinex/material_templates/surface_depth.slang");
			return *this;
		}

		bool Geometry::is_material_compatible(const Material* material)
		{
			return material->domain == MaterialDomain::Surface && material->blend_mode.is_opaque();
		}

		Geometry& Geometry::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
		{
			Super::modify_shader_compilation_env(env);
			env->add_module("trinex/material_templates/surface_geometry.slang");
			return *this;
		}

		Geometry& Geometry::begin(Renderer* renderer, RHIContext* ctx)
		{
			const ViewMode view_mode = renderer->view_mode();

			RHIRasterizerState rasterizer;
			rasterizer.polygon_mode = view_mode == ViewMode::Wireframe ? RHIPolygonMode::Line : RHIPolygonMode::Fill;

			ctx->rasterizer_state(rasterizer);
			ctx->blending_state(RHIBlendingState());
			return *this;
		}

		bool Geometry::blending_state(RHIBlendingState& state)
		{
			return false;
		}

		bool Translucent::is_material_compatible(const Material* material)
		{
			return material->domain == MaterialDomain::Surface && material->blend_mode.is_translucent();
		}

		Translucent& Translucent::modify_shader_compilation_env(ShaderCompilationEnvironment* env)
		{
			Super::modify_shader_compilation_env(env);
			env->add_module("trinex/material_templates/surface_translucent.slang");
			return *this;
		}

		Translucent& Translucent::begin(Renderer* renderer, RHIContext* ctx)
		{
			RHIRasterizerState rasterizer;
			rasterizer.polygon_mode = RHIPolygonMode::Fill;
			rasterizer.cull_mode    = RHICullMode::Back;
			ctx->rasterizer_state(rasterizer);
			return *this;
		}
	}// namespace RenderPasses
}// namespace Trinex
