#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>

namespace Engine::Pipelines
{
	static FORCE_INLINE Vector4i rect_to_vec4(const Rect2D& rect)
	{
		return {rect.pos.x, rect.pos.y, rect.size.x, rect.size.y};
	}

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
			sampler = Sampler(SamplerFilter::Point).rhi_sampler();

		rhi_bind();

		rhi->bind_srv(src, m_src->location, sampler);
		rhi->bind_uav(dst, m_dst->location);

		rhi->update_scalar_parameter(&kernel, m_kernel_size);
		rhi->update_scalar_parameter(&sigma, m_sigma);

		// Shader uses 8x8x1 threads per group
		Vector2u groups = {(dst_size.x + 7) / 8, (dst_size.y + 7) / 8};
		rhi->dispatch(groups.x, groups.y, 1);
	}

	trinex_implement_pipeline(Blit2D, "[shaders_dir]:/TrinexEngine/trinex/compute/blit.slang", ShaderType::Compute)
	{
		m_src  = find_param_info("src");
		m_dst  = find_param_info("dst");
		m_args = find_param_info("args");
	}

	void Blit2D::blit(RHI_ShaderResourceView* src, RHI_UnorderedAccessView* dst, const Rect2D& src_rect, const Rect2D& dst_rect,
	                  uint_t level, Swizzle swizzle)
	{
		struct ShaderArgs {
			alignas(16) Vector4i src_rect;
			alignas(16) Vector4i dst_rect;
			alignas(16) Vector4u swizzle;
			alignas(4) uint32_t level;
		};

		ShaderArgs shader_args;
		shader_args.src_rect = rect_to_vec4(src_rect);
		shader_args.dst_rect = rect_to_vec4(dst_rect);
		shader_args.swizzle  = swizzle;
		shader_args.level    = level;

		rhi_bind();

		rhi->bind_srv(src, m_src->location);
		rhi->bind_uav(dst, m_dst->location);

		rhi->update_scalar_parameter(&shader_args, sizeof(shader_args), m_args);

		// Shader uses 8x8x1 threads per group
		Vector2u groups = {(glm::abs(dst_rect.size.x) + 7) / 8, (glm::abs(dst_rect.size.y) + 7) / 8};
		rhi->dispatch(groups.x, groups.y, 1);
	}

	trinex_implement_pipeline(BatchedLines, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_lines.slang",
	                          ShaderType::Vertex | ShaderType::Geometry | ShaderType::Fragment)
	{
		input_assembly.primitive_topology = PrimitiveTopology::LineList;
		color_blending.enable             = false;

		m_scene_view = find_param_info("scene_view");
	}

	void BatchedLines::apply(Renderer* renderer)
	{
		rhi_bind();
		rhi->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->location);
	}

	trinex_implement_pipeline(BatchedTriangles, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_triangles.slang",
	                          ShaderType::Vertex | ShaderType::Fragment)
	{
		input_assembly.primitive_topology = PrimitiveTopology::TriangleList;
		color_blending.enable             = true;
	}

	void DeferredLightPipeline::initialize()
	{
		scene_view         = find_param_info("scene_view");
		base_color_texture = find_param_info("base_color_texture");
		normal_texture     = find_param_info("normal_texture");
		emissive_texture   = find_param_info("emissive_texture");
		msra_texture       = find_param_info("msra_texture");
		depth_texture      = find_param_info("depth_texture");
		parameters         = find_param_info("parameters");

		depth_test.enable       = false;
		depth_test.write_enable = false;
		color_blending.enable   = true;

		color_blending.src_color_func = BlendFunc::One;
		color_blending.dst_color_func = BlendFunc::One;
		color_blending.color_op       = BlendOp::Max;

		color_blending.src_alpha_func = BlendFunc::One;
		color_blending.dst_alpha_func = BlendFunc::One;
		color_blending.alpha_op       = BlendOp::Max;
	}

	trinex_implement_pipeline(DeferredPointLightShadowed, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
	                          ShaderType::BasicGraphics)
	{
		Super::initialize();
	}

	DeferredPointLightShadowed& DeferredPointLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/point_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredPointLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
	                          ShaderType::BasicGraphics)
	{
		Super::initialize();
	}

	DeferredPointLight& DeferredPointLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/point_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredSpotLightShadowed, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
	                          ShaderType::BasicGraphics)
	{
		Super::initialize();
	}

	DeferredSpotLightShadowed& DeferredSpotLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/spot_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredSpotLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
	                          ShaderType::BasicGraphics)
	{
		Super::initialize();
	}

	DeferredSpotLight& DeferredSpotLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/spot_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredDirectionalLightShadowed,
	                          "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang", ShaderType::BasicGraphics)
	{
		Super::initialize();
	}

	DeferredDirectionalLightShadowed& DeferredDirectionalLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/directional_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredDirectionalLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang",
	                          ShaderType::BasicGraphics)
	{
		Super::initialize();
	}

	DeferredDirectionalLight& DeferredDirectionalLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/directional_light.slang");
		return *this;
	}

	trinex_implement_pipeline(AmbientLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/ambient_light.slang",
	                          ShaderType::BasicGraphics)
	{
		//setup_lighting_pipeline_state(this);

		scene_view    = find_param_info("scene_view");
		base_color    = find_param_info("base_color");
		msra          = find_param_info("msra");
		ambient_color = find_param_info("ambient_color");
	}
}// namespace Engine::Pipelines
