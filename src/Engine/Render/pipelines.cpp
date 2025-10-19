#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/exception.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <random>

namespace Engine::Pipelines
{
	static inline void push_context_state(Pipeline* pipeline, RHIContext* ctx)
	{
		ctx->push_depth_state(RHIDepthState(false));
		ctx->push_stencil_state(RHIStencilState(false));
		ctx->push_primitive_topology(RHIPrimitiveTopology::TriangleList);
		ctx->push_cull_mode(RHICullMode::None);
		ctx->push_pipeline(pipeline->rhi_pipeline());
	}

	static inline void pop_context_state(RHIContext* ctx)
	{
		ctx->pop_depth_state();
		ctx->pop_stencil_state();
		ctx->pop_primitive_topology();
		ctx->pop_cull_mode();
		ctx->pop_pipeline();
	}

	trinex_implement_pipeline(GaussianBlur, "[shaders_dir]:/TrinexEngine/trinex/graphics/gaussian_blur.slang")
	{
		m_source = find_parameter("source");
		m_args   = find_parameter("args");
	}

	void GaussianBlur::blur(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset, Vector2f inv_size, Vector2f direction,
	                        float sigma, float radius, Swizzle swizzle, RHISampler* sampler)
	{
		struct Args {
			alignas(16) Vector4u swizzle;
			alignas(8) Vector2f offset;
			alignas(8) Vector2f inv_size;
			alignas(8) Vector2f direction;
			alignas(4) float sigma;
			alignas(4) float radius;
		};

		Args args;
		args.swizzle   = swizzle;
		args.offset    = offset;
		args.inv_size  = inv_size;
		args.direction = Math::normalize(direction);
		args.sigma     = sigma;
		args.radius    = radius;

		if (sampler == nullptr)
			sampler = RHIBilinearSampler::static_sampler();

		push_context_state(this, ctx);

		ctx->bind_srv(src, m_source->binding);
		ctx->bind_sampler(sampler, m_source->binding);
		ctx->update_scalar(&args, sizeof(args), m_args);

		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(Blit2D, "[shaders_dir]:/TrinexEngine/trinex/graphics/blit.slang")
	{
		m_source = find_parameter("source");
		m_args   = find_parameter("args");
	}

	void Blit2D::blit(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset, Vector2f inv_size, Swizzle swizzle,
	                  RHISampler* sampler)
	{
		struct ShaderArgs {
			alignas(8) Vector2f offset;
			alignas(8) Vector2f inv_size;
			alignas(16) Vector4u swizzle;
		};

		if (sampler == nullptr)
			sampler = RHIPointSampler::static_sampler();

		ShaderArgs shader_args;
		shader_args.offset   = offset;
		shader_args.inv_size = inv_size;
		shader_args.swizzle  = swizzle;

		push_context_state(this, ctx);

		ctx->bind_srv(src, m_source->binding);
		ctx->bind_sampler(sampler, m_source->binding);
		ctx->update_scalar(&shader_args, sizeof(shader_args), m_args);

		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(BatchedLines, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_lines.slang")
	{
		m_projview = find_parameter("projview");
		m_viewport = find_parameter("viewport");
	}

	trinex_implement_pipeline(BatchedTriangles, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_triangles.slang") {}

	trinex_implement_pipeline(DeferredLighting, "[shaders_dir]:/TrinexEngine/trinex/lighting/deferred.slang")
	{
		scene_view         = find_parameter("scene_view");
		base_color_texture = find_parameter("base_color_texture");
		normal_texture     = find_parameter("normal_texture");
		emissive_texture   = find_parameter("emissive_texture");
		msra_texture       = find_parameter("msra_texture");
		depth_texture      = find_parameter("depth_texture");

		screen_sampler = find_parameter("screen_sampler");
		shadow_sampler = find_parameter("shadow_sampler");

		ranges   = find_parameter("ranges");
		clusters = find_parameter("clusters");
		lights   = find_parameter("lights");
		shadows  = find_parameter("shadows");
	}

	trinex_implement_pipeline(AmbientLight, "[shaders_dir]:/TrinexEngine/trinex/lighting/ambient.slang")
	{
		//setup_lighting_pipeline_state(this);

		scene_view    = find_parameter("scene_view");
		base_color    = find_parameter("base_color");
		msra          = find_parameter("msra");
		ambient_color = find_parameter("ambient_color");
	}

	trinex_implement_pipeline(TonemappingACES, "[shaders_dir]:/TrinexEngine/trinex/graphics/tonemapping.slang")
	{
		m_hdr_target = find_parameter("hdr_scene");
		m_scene_view = find_parameter("scene_view");
	}

	TonemappingACES& TonemappingACES::apply(RHIContext* ctx, Renderer* renderer)
	{
		push_context_state(this, ctx);

		ctx->bind_render_target1(renderer->scene_color_ldr_target()->as_rtv());
		ctx->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		ctx->bind_srv(renderer->scene_color_hdr_target()->as_srv(), m_hdr_target->binding);
		ctx->bind_sampler(RHIPointSampler::static_sampler(), m_hdr_target->binding);
		ctx->draw(6, 0);

		pop_context_state(ctx);

		return *this;
	}

	trinex_implement_pipeline(SSR, "[shaders_dir]:/TrinexEngine/trinex/graphics/ssr.slang")
	{
		scene_view   = find_parameter("scene_view");
		scene_color  = find_parameter("scene_color");
		scene_normal = find_parameter("scene_normal");
		scene_depth  = find_parameter("scene_depth");
		sampler      = find_parameter("sampler");
	}

	trinex_implement_pipeline(SSAO, "[shaders_dir]:/TrinexEngine/trinex/graphics/ssao.slang")
	{
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

	SSAO& SSAO::render(RHIContext* ctx, Renderer* renderer, float intensity, float bias, float power, float radius,
	                   float fade_out_distance, float fade_out_radius, uint_t samples)
	{
		create_samples_buffer(samples);

		ctx->bind_pipeline(rhi_pipeline());

		SSAOArguments args;
		args.noise_scale       = renderer->scene_view().view_size() / Vector2i(4, 4);
		args.intensity         = intensity;
		args.bias              = bias;
		args.power             = power;
		args.radius            = radius;
		args.fade_out_distance = fade_out_distance;
		args.fade_out_radius   = fade_out_radius;
		args.samples           = samples;

		ctx->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		ctx->update_scalar(&args, sizeof(args), 0, m_args->binding);
		ctx->bind_srv(renderer->scene_depth_target()->as_srv(), m_scene_depth->binding);
		ctx->bind_srv(renderer->normal_target()->as_srv(), m_scene_normal->binding);
		ctx->bind_srv(DefaultResources::Textures::noise4x4->rhi_srv(), m_noise->binding);
		ctx->bind_srv(m_samples_buffer->as_srv(), m_samples->binding);
		ctx->bind_sampler(RHIBilinearWrapSampler::static_sampler(), m_sampler->binding);

		ctx->draw(6, 0);
		return *this;
	}

	trinex_implement_pipeline(ClusterInitialize, "[shaders_dir]:/TrinexEngine/trinex/cluster/initialize.slang")
	{
		m_scene_view = find_parameter("scene_view");
		m_clusters   = find_parameter("clusters");
	}

	RHIBuffer* ClusterInitialize::create_clusters_buffer()
	{
		static constexpr size_t cluster_size        = 576;
		static constexpr RHIBufferCreateFlags flags = RHIBufferCreateFlags::UnorderedAccess |
		                                              RHIBufferCreateFlags::ShaderResource |
		                                              RHIBufferCreateFlags::StructuredBuffer;

		size_t buffer_size = 16 * 9 * 24 * cluster_size;
		return RHIBufferPool::global_instance()->request_transient_buffer(buffer_size, flags);
	}

	ClusterInitialize& ClusterInitialize::build(RHIContext* ctx, RHIBuffer* clusters, Renderer* renderer)
	{
		ctx->bind_pipeline(rhi_pipeline());
		ctx->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		ctx->bind_uav(clusters->as_uav(), m_clusters->binding);
		ctx->dispatch(16, 9, 24);
		return *this;
	}

	trinex_implement_pipeline(ClusterLightCulling, "[shaders_dir]:/TrinexEngine/trinex/cluster/light_culling.slang")
	{
		m_scene_view = find_parameter("scene_view");
		m_clusters   = find_parameter("clusters");
		m_lights     = find_parameter("lights");
		m_ranges     = find_parameter("ranges");
	}

	ClusterLightCulling& ClusterLightCulling::cull(RHIContext* ctx, Renderer* renderer, RHIBuffer* clusters, RHIBuffer* lights,
	                                               const LightRenderRanges& ranges)
	{
		ctx->bind_pipeline(rhi_pipeline());
		ctx->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		ctx->bind_uav(clusters->as_uav(), m_clusters->binding);
		ctx->bind_srv(lights->as_srv(), m_lights->binding);
		ctx->update_scalar(&ranges, m_ranges);
		ctx->dispatch(27, 1, 1);
		return *this;
	}
}// namespace Engine::Pipelines
