#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Graphics/material_compiler.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>

namespace Engine::Pipelines
{
	trinex_implement_pipeline(GaussianBlur, "[shaders_dir]:/TrinexEngine/trinex/compute/gaussian_blur.slang", ShaderType::Compute)
	{
		m_src         = find_param_info("input");
		m_dst         = find_param_info("output");
		m_sigma       = find_param_info("sigma");
		m_kernel_size = find_param_info("kernel_size");
	}

	void GaussianBlur::blur(RHI_ShaderResourceView* src, RHI_UnorderedAccessView* dst, const Vector2u& dst_size, int32_t kernel,
							float sigma, RHI_Sampler* sampler)
	{
		kernel = glm::abs(kernel);
		sigma  = glm::abs(sigma);

		if (sampler == nullptr)
			sampler = DefaultResources::Samplers::default_sampler->rhi_sampler();

		rhi_bind();

		src->bind_combined(m_src->location, sampler);
		dst->bind(m_dst->location);

		rhi->update_scalar_parameter(&kernel, sizeof(kernel), m_kernel_size->offset, m_kernel_size->location);
		rhi->update_scalar_parameter(&sigma, sizeof(sigma), m_sigma->offset, m_sigma->location);

		// Shader uses 8x8x1 threads per group
		Vector2u groups = {(dst_size.x + 7) / 8, (dst_size.y + 7) / 8};
		rhi->dispatch(groups.x, groups.y, 1);
	}

	trinex_implement_pipeline(BatchedLines, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_lines.slang",
							  ShaderType::Vertex | ShaderType::Geometry | ShaderType::Fragment)
	{
		input_assembly.primitive_topology = PrimitiveTopology::LineList;
		color_blending.enable             = true;

		m_globals = find_param_info("globals");
	}

	void BatchedLines::apply(SceneRenderer* renderer)
	{
		rhi_bind();
		renderer->bind_global_parameters(m_globals->location);
	}

	trinex_implement_pipeline(BatchedTriangles, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_triangles.slang",
							  ShaderType::Vertex | ShaderType::Fragment)
	{
		input_assembly.primitive_topology = PrimitiveTopology::TriangleList;
		color_blending.enable             = true;
	}

	static void setup_lighting_pipeline_state(GraphicsPipeline* pipeline)
	{
		pipeline->depth_test.enable       = false;
		pipeline->depth_test.write_enable = false;
		pipeline->color_blending.enable   = true;

		pipeline->color_blending.src_color_func = BlendFunc::One;
		pipeline->color_blending.dst_color_func = BlendFunc::One;
		pipeline->color_blending.color_op       = BlendOp::Max;

		pipeline->color_blending.src_alpha_func = BlendFunc::One;
		pipeline->color_blending.dst_alpha_func = BlendFunc::One;
		pipeline->color_blending.alpha_op       = BlendOp::Max;
	}

	trinex_implement_pipeline(DeferredPointLightShadowed, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
							  ShaderType::BasicGraphics)
	{
		setup_lighting_pipeline_state(this);

		globals             = find_param_info("globals");
		base_color_texture  = find_param_info("base_color_texture");
		normal_texture      = find_param_info("normal_texture");
		emissive_texture    = find_param_info("emissive_texture");
		msra_texture        = find_param_info("msra_texture");
		depth_texture       = find_param_info("depth_texture");
		shadow_map_texture  = find_param_info("shadow_map_texture");
		shadow_map_projview = find_param_info("shadow_map_projview");

		color             = find_param_info("light_data.color");
		intensivity       = find_param_info("light_data.intensivity");
		depth_bias        = find_param_info("light_data.depth_bias");
		slope_scale       = find_param_info("light_data.slope_scale");
		location          = find_param_info("light_data.location");
		radius            = find_param_info("light_data.radius");
		fall_off_exponent = find_param_info("light_data.fall_off_exponent");
	}

	DeferredPointLightShadowed& DeferredPointLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_definition_nocopy("TRINEX_POINT_LIGHT", "1");
		env->add_definition_nocopy("TRINEX_DEFERRED_LIGHTING", "1");
		ShadowedLightingPass::static_info()->modify_shader_compilation_env(env);
		return *this;
	}

	trinex_implement_pipeline(DeferredPointLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
							  ShaderType::BasicGraphics)
	{
		setup_lighting_pipeline_state(this);

		globals            = find_param_info("globals");
		base_color_texture = find_param_info("base_color_texture");
		normal_texture     = find_param_info("normal_texture");
		emissive_texture   = find_param_info("emissive_texture");
		msra_texture       = find_param_info("msra_texture");
		depth_texture      = find_param_info("depth_texture");
	}

	DeferredPointLight& DeferredPointLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_definition_nocopy("TRINEX_POINT_LIGHT", "1");
		env->add_definition_nocopy("TRINEX_DEFERRED_LIGHTING", "1");
		LightingPass::static_info()->modify_shader_compilation_env(env);
		return *this;
	}

	trinex_implement_pipeline(DeferredSpotLightShadowed, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
							  ShaderType::BasicGraphics)
	{
		setup_lighting_pipeline_state(this);

		globals             = find_param_info("globals");
		base_color_texture  = find_param_info("base_color_texture");
		normal_texture      = find_param_info("normal_texture");
		emissive_texture    = find_param_info("emissive_texture");
		msra_texture        = find_param_info("msra_texture");
		depth_texture       = find_param_info("depth_texture");
		shadow_map_texture  = find_param_info("shadow_map_texture");
		shadow_map_projview = find_param_info("shadow_map_projview");

		color             = find_param_info("light_data.color");
		intensivity       = find_param_info("light_data.intensivity");
		depth_bias        = find_param_info("light_data.depth_bias");
		slope_scale       = find_param_info("light_data.slope_scale");
		location          = find_param_info("light_data.location");
		radius            = find_param_info("light_data.radius");
		fall_off_exponent = find_param_info("light_data.fall_off_exponent");
		direction         = find_param_info("light_data.direction");
		spot_angles       = find_param_info("light_data.spot_angles");
	}

	DeferredSpotLightShadowed& DeferredSpotLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_definition_nocopy("TRINEX_SPOT_LIGHT", "1");
		env->add_definition_nocopy("TRINEX_DEFERRED_LIGHTING", "1");
		ShadowedLightingPass::static_info()->modify_shader_compilation_env(env);
		return *this;
	}

	trinex_implement_pipeline(DeferredSpotLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
							  ShaderType::BasicGraphics)
	{
		setup_lighting_pipeline_state(this);

		globals            = find_param_info("globals");
		base_color_texture = find_param_info("base_color_texture");
		normal_texture     = find_param_info("normal_texture");
		emissive_texture   = find_param_info("emissive_texture");
		msra_texture       = find_param_info("msra_texture");
		depth_texture      = find_param_info("depth_texture");
	}

	DeferredSpotLight& DeferredSpotLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_definition_nocopy("TRINEX_SPOT_LIGHT", "1");
		env->add_definition_nocopy("TRINEX_DEFERRED_LIGHTING", "1");
		LightingPass::static_info()->modify_shader_compilation_env(env);
		return *this;
	}

	trinex_implement_pipeline(DeferredDirectionalLightShadowed,
							  "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang", ShaderType::BasicGraphics)
	{
		setup_lighting_pipeline_state(this);

		globals             = find_param_info("globals");
		base_color_texture  = find_param_info("base_color_texture");
		normal_texture      = find_param_info("normal_texture");
		emissive_texture    = find_param_info("emissive_texture");
		msra_texture        = find_param_info("msra_texture");
		depth_texture       = find_param_info("depth_texture");
		shadow_map_texture  = find_param_info("shadow_map_texture");
		shadow_map_projview = find_param_info("shadow_map_projview");

		color       = find_param_info("light_data.color");
		intensivity = find_param_info("light_data.intensivity");
		depth_bias  = find_param_info("light_data.depth_bias");
		slope_scale = find_param_info("light_data.slope_scale");
		direction   = find_param_info("light_data.direction");
	}

	DeferredDirectionalLightShadowed& DeferredDirectionalLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_definition_nocopy("TRINEX_DIRECTIONAL_LIGHT", "1");
		env->add_definition_nocopy("TRINEX_DEFERRED_LIGHTING", "1");
		ShadowedLightingPass::static_info()->modify_shader_compilation_env(env);
		return *this;
	}

	trinex_implement_pipeline(DeferredDirectionalLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
							  ShaderType::BasicGraphics)
	{
		setup_lighting_pipeline_state(this);

		globals            = find_param_info("globals");
		base_color_texture = find_param_info("base_color_texture");
		normal_texture     = find_param_info("normal_texture");
		emissive_texture   = find_param_info("emissive_texture");
		msra_texture       = find_param_info("msra_texture");
		depth_texture      = find_param_info("depth_texture");
	}

	DeferredDirectionalLight& DeferredDirectionalLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_definition_nocopy("TRINEX_DIRECTIONAL_LIGHT", "1");
		env->add_definition_nocopy("TRINEX_DEFERRED_LIGHTING", "1");
		LightingPass::static_info()->modify_shader_compilation_env(env);
		return *this;
	}
}// namespace Engine::Pipelines
