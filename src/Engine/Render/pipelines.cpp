#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>
#include <RHI/rhi.hpp>

namespace Engine::Pipelines
{
	static FORCE_INLINE Vector4i rect_to_vec4(const RHIRect& rect)
	{
		return {rect.pos.x, rect.pos.y, rect.size.x, rect.size.y};
	}

	trinex_implement_pipeline(GaussianBlur, "[shaders_dir]:/TrinexEngine/trinex/compute/gaussian_blur.slang")
	{
		m_src         = find_parameter("input");
		m_dst         = find_parameter("output");
		m_sigma       = find_parameter("sigma");
		m_kernel_size = find_parameter("kernel_size");
	}

	void GaussianBlur::blur(RHIShaderResourceView* src, RHIUnorderedAccessView* dst, const Vector2u& dst_size, int32_t kernel,
	                        float sigma, RHISampler* sampler)
	{
		kernel = glm::abs(kernel);
		sigma  = glm::abs(sigma);

		if (sampler == nullptr)
			sampler = Sampler(RHISamplerFilter::Point).rhi_sampler();

		rhi_bind();

		rhi->bind_srv(src, m_src->binding);
		rhi->bind_sampler(sampler, m_src->binding);
		rhi->bind_uav(dst, m_dst->binding);

		rhi->update_scalar_parameter(&kernel, m_kernel_size);
		rhi->update_scalar_parameter(&sigma, m_sigma);

		// Shader uses 8x8x1 threads per group
		Vector2u groups = {(dst_size.x + 7) / 8, (dst_size.y + 7) / 8};
		rhi->dispatch(groups.x, groups.y, 1);
	}

	trinex_implement_pipeline(Blit2D, "[shaders_dir]:/TrinexEngine/trinex/compute/blit.slang")
	{
		m_src  = find_parameter("src");
		m_dst  = find_parameter("dst");
		m_args = find_parameter("args");
	}

	void Blit2D::blit(RHIShaderResourceView* src, RHIUnorderedAccessView* dst, const RHIRect& src_rect, const RHIRect& dst_rect,
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

		rhi->bind_srv(src, m_src->binding);
		rhi->bind_uav(dst, m_dst->binding);

		rhi->update_scalar_parameter(&shader_args, sizeof(shader_args), m_args);

		// Shader uses 8x8x1 threads per group
		Vector2u groups = {(glm::abs(dst_rect.size.x) + 7) / 8, (glm::abs(dst_rect.size.y) + 7) / 8};
		rhi->dispatch(groups.x, groups.y, 1);
	}

	trinex_implement_pipeline(BatchedLines, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_lines.slang")
	{
		color_blending.enable = false;
		m_projview            = find_parameter("projview");
		m_viewport            = find_parameter("viewport");
	}

	trinex_implement_pipeline(BatchedTriangles, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_triangles.slang")
	{
		color_blending.enable = true;
	}

	void DeferredLightPipeline::initialize()
	{
		scene_view         = find_parameter("scene_view");
		base_color_texture = find_parameter("base_color_texture");
		normal_texture     = find_parameter("normal_texture");
		msra_texture       = find_parameter("msra_texture");
		depth_texture      = find_parameter("depth_texture");
		parameters         = find_parameter("parameters");

		shadow_map      = find_parameter("shadow_map");
		shadow_projview = find_parameter("shadow_projview");

		depth_test.enable       = false;
		depth_test.write_enable = false;

		color_blending.enable         = true;
		color_blending.src_color_func = RHIBlendFunc::One;
		color_blending.dst_color_func = RHIBlendFunc::One;
		color_blending.color_op       = RHIBlendOp::Add;

		color_blending.write_mask = RHIColorComponent::R | RHIColorComponent::G | RHIColorComponent::B;
	}

	trinex_implement_pipeline(DeferredPointLightShadowed, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang")
	{
		Super::initialize();
	}

	DeferredPointLightShadowed& DeferredPointLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/lights/point_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredPointLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang")
	{
		Super::initialize();
	}

	DeferredPointLight& DeferredPointLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/lights/point_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredSpotLightShadowed, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang")
	{
		Super::initialize();
	}

	DeferredSpotLightShadowed& DeferredSpotLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/lights/shadowed_spot_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredSpotLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang")
	{
		Super::initialize();
	}

	DeferredSpotLight& DeferredSpotLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/lights/spot_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredDirectionalLightShadowed,
	                          "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang")
	{
		Super::initialize();
	}

	DeferredDirectionalLightShadowed& DeferredDirectionalLightShadowed::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/lights/directional_light.slang");
		return *this;
	}

	trinex_implement_pipeline(DeferredDirectionalLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/deferred_light.slang")
	{
		Super::initialize();
	}

	DeferredDirectionalLight& DeferredDirectionalLight::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		Super::modify_compilation_env(env);
		env->add_module("trinex/lights/directional_light.slang");
		return *this;
	}

	trinex_implement_pipeline(AmbientLight, "[shaders_dir]:/TrinexEngine/trinex/graphics/ambient_light.slang")
	{
		//setup_lighting_pipeline_state(this);

		scene_view    = find_parameter("scene_view");
		base_color    = find_parameter("base_color");
		msra          = find_parameter("msra");
		ambient_color = find_parameter("ambient_color");
	}

	trinex_implement_pipeline(TonemappingACES, "[shaders_dir]:/TrinexEngine/trinex/graphics/tonemapping.slang")
	{
		depth_test.enable       = false;
		depth_test.write_enable = false;
		stencil_test.enable     = false;

		m_hdr_target = find_parameter("hdr_scene");
		m_exposure   = find_parameter("exposure");
		m_scene_view = find_parameter("scene_view");
	}

	TonemappingACES& TonemappingACES::apply(Renderer* renderer)
	{
		rhi->bind_render_target1(renderer->scene_color_ldr_target()->as_rtv());
		rhi_bind();

		float EV = 0.f;
		rhi->update_scalar_parameter(&EV, m_exposure);
		rhi->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		rhi->bind_srv(renderer->scene_color_hdr_target()->as_srv(), m_hdr_target->binding);
		rhi->draw(6, 0);

		return *this;
	}
}// namespace Engine::Pipelines
