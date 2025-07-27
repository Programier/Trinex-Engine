#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/exception.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture_2D.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <random>

namespace Engine::Pipelines
{
	static FORCE_INLINE Vector4i rect_to_vec4(const RHIRect& rect)
	{
		return {rect.pos.x, rect.pos.y, rect.size.x, rect.size.y};
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
		m_scene_view = find_parameter("scene_view");
	}

	TonemappingACES& TonemappingACES::apply(Renderer* renderer)
	{
		rhi->bind_render_target1(renderer->scene_color_ldr_target()->as_rtv());
		rhi_bind();

		rhi->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		rhi->bind_srv(renderer->scene_color_hdr_target()->as_srv(), m_hdr_target->binding);
		rhi->bind_sampler(RHIPointSampler::static_sampler(), m_hdr_target->binding);
		rhi->draw(6, 0);

		return *this;
	}

	trinex_implement_pipeline(SSR, "[shaders_dir]:/TrinexEngine/trinex/graphics/ssr.slang")
	{
		depth_test.enable       = false;
		depth_test.write_enable = false;
		stencil_test.enable     = false;
		color_blending.enable   = false;

		scene_view   = find_parameter("scene_view");
		scene_color  = find_parameter("scene_color");
		scene_normal = find_parameter("scene_normal");
		scene_depth  = find_parameter("scene_depth");
		sampler      = find_parameter("sampler");
	}

	trinex_implement_pipeline(SSAO, "[shaders_dir]:/TrinexEngine/trinex/graphics/ssao.slang")
	{
		depth_test.enable       = false;
		depth_test.write_enable = false;
		stencil_test.enable     = false;
		color_blending.enable   = true;

		color_blending.color_op       = RHIBlendOp::Add;
		color_blending.src_color_func = RHIBlendFunc::Zero;
		color_blending.dst_color_func = RHIBlendFunc::Zero;

		color_blending.alpha_op       = RHIBlendOp::Add;
		color_blending.src_alpha_func = RHIBlendFunc::DstAlpha;
		color_blending.dst_alpha_func = RHIBlendFunc::Zero;
		color_blending.write_mask     = RHIColorComponent::A;

		m_scene_view   = find_parameter("scene_view");
		m_scene_depth  = find_parameter("scene_depth");
		m_scene_normal = find_parameter("scene_normal");
		m_noise        = find_parameter("noise_texture");
		m_sampler      = find_parameter("sampler");
		m_args         = find_parameter("args");
		m_samples      = find_parameter("samples");

		DestroyController([]() {
			RHIBuffer* buffer = SSAO::instance()->m_samples_buffer;
			if (buffer)
			{
				RHIObject::static_release(buffer);
			}
		});
	}

	SSAO& SSAO::create_samples_buffer(size_t count)
	{
		if (m_samples_count >= count)
			return *this;

		if (m_samples_buffer)
			m_samples_buffer->release();

		StackByteAllocator::Mark mark;
		Vector3f* kernel = StackAllocator<Vector3f>::allocate(count);

		std::uniform_real_distribution<float> random(0.0, 1.0);
		std::default_random_engine generator;

		for (size_t i = 0; i < count; ++i)
		{
			Vector3f& sample = kernel[i];

			sample.x = random(generator) * 2.0f - 1.f;
			sample.y = random(generator) * 2.0f - 1.f;
			sample.z = random(generator) * 2.0f - 1.f;

			sample = glm::normalize(sample);
			sample *= random(generator);
			float factor = static_cast<float>(i) / static_cast<float>(count);
			sample *= glm::mix(0.1f, 1.0f, factor * factor);
		}

		m_samples_buffer = rhi->create_buffer(count * sizeof(Vector3f), reinterpret_cast<byte*>(kernel),
		                                      RHIBufferCreateFlags::StructuredBuffer | RHIBufferCreateFlags::ShaderResource);
		return *this;
	}

	struct SSAOArguments {
		Vector2f noise_scale;
		float intensity;
		float bias;
		float power;
		float radius;
		float fade_out_distance;
		float fade_out_radius;
		uint32_t samples;
	};

	SSAO& SSAO::apply(Renderer* renderer, float intensity, float bias, float power, float radius, float fade_out_distance,
	                  float fade_out_radius, uint_t samples)
	{
		create_samples_buffer(samples);

		rhi_bind();

		SSAOArguments args;
		args.noise_scale       = renderer->scene_view().view_size() / Vector2i(4, 4);
		args.intensity         = intensity;
		args.bias              = bias;
		args.power             = power;
		args.radius            = radius;
		args.fade_out_distance = fade_out_distance;
		args.fade_out_radius   = fade_out_radius;
		args.samples           = samples;

		rhi->bind_render_target1(renderer->msra_target()->as_rtv());

		rhi->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		rhi->update_scalar_parameter(&args, sizeof(args), 0, m_args->binding);
		rhi->bind_srv(renderer->scene_depth_target()->as_srv(), m_scene_depth->binding);
		rhi->bind_srv(renderer->normal_target()->as_srv(), m_scene_normal->binding);
		rhi->bind_srv(DefaultResources::Textures::noise4x4->rhi_srv(), m_noise->binding);
		rhi->bind_srv(m_samples_buffer->as_srv(), m_samples->binding);
		rhi->bind_sampler(RHIBilinearWrapSampler::static_sampler(), m_sampler->binding);

		rhi->draw(6, 0);
		return *this;
	}
}// namespace Engine::Pipelines
