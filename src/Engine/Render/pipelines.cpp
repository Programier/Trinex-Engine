#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/math/random.hpp>
#include <Engine/Render/frame_history.hpp>
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

namespace Engine::Pipelines
{
	static inline void push_context_state(Pipeline* pipeline, RHIContext* ctx)
	{
		ctx->depth_state(RHIDepthState(false));
		ctx->stencil_state(RHIStencilState(false));
		ctx->push_primitive_topology(RHIPrimitiveTopology::TriangleList);
		ctx->push_cull_mode(RHICullMode::None);
		ctx->bind_pipeline(pipeline->rhi_pipeline());
	}

	static inline void pop_context_state(RHIContext* ctx)
	{
		ctx->pop_primitive_topology();
		ctx->pop_cull_mode();
	}

	trinex_implement_pipeline(GaussianBlur, "[shaders]:/TrinexEngine/trinex/graphics/gaussian_blur.slang")
	{
		m_source = find_parameter("source");
		m_args   = find_parameter("args");
	}

	void GaussianBlur::blur(RHIContext* ctx, RHIShaderResourceView* src, Vector2f direction, float sigma, float radius,
	                        Swizzle swizzle, RHISampler* sampler, Vector2f offset, Vector2f size)
	{
		struct Args {
			alignas(8) Vector2f offset;
			alignas(8) Vector2f size;
			alignas(8) Vector2f direction;
			alignas(4) float sigma;
			alignas(4) float radius;
			alignas(4) Swizzle swizzle;
		};

		Args args;
		args.offset    = offset;
		args.size      = size;
		args.direction = direction;
		args.sigma     = sigma;
		args.radius    = radius;
		args.swizzle   = swizzle;

		if (sampler == nullptr)
			sampler = RHIBilinearSampler::static_sampler();

		auto self = instance();
		push_context_state(self, ctx);

		ctx->bind_srv(src, self->m_source->binding);
		ctx->bind_sampler(sampler, self->m_source->binding);
		ctx->update_scalar(&args, sizeof(args), self->m_args);

		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(Blit2D, "[shaders]:/TrinexEngine/trinex/graphics/blit.slang")
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
			alignas(4) Swizzle swizzle;
		};

		if (sampler == nullptr)
			sampler = RHIPointSampler::static_sampler();

		ShaderArgs shader_args;
		shader_args.offset   = offset;
		shader_args.inv_size = inv_size;
		shader_args.swizzle  = swizzle;

		auto self = instance();

		push_context_state(self, ctx);

		ctx->bind_srv(src, self->m_source->binding);
		ctx->bind_sampler(sampler, self->m_source->binding);
		ctx->update_scalar(&shader_args, sizeof(shader_args), self->m_args);

		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(Passthrow, "[shaders]:/TrinexEngine/trinex/graphics/passthrow.slang")
	{
		m_scene = find_parameter("scene");
		m_args  = find_parameter("args");
	}

	void Passthrow::passthrow(RHIContext* ctx, RHIShaderResourceView* src, Swizzle swizzle, Vector2f offset, Vector2f size,
	                          RHISampler* sampler)
	{
		struct Args {
			alignas(8) Vector2f offset;
			alignas(8) Vector2f size;
			alignas(4) Swizzle swizzle;
		};

		Args args;
		args.offset  = offset;
		args.size    = size;
		args.swizzle = swizzle;

		auto self = instance();
		push_context_state(self, ctx);

		if (sampler == nullptr)
			sampler = RHIBilinearSampler::static_sampler();

		ctx->bind_srv(src, self->m_scene->binding);
		ctx->bind_sampler(sampler, self->m_scene->binding);
		ctx->update_scalar(&args, sizeof(args), self->m_args);
		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(Downsample, "[shaders]:/TrinexEngine/trinex/graphics/downsample.slang")
	{
		m_scene = find_parameter("scene");
		m_args  = find_parameter("args");
	}

	void Downsample::downsample(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset, Vector2f size)
	{
		push_context_state(this, ctx);

		struct Args {
			alignas(8) Vector2f offset;
			alignas(8) Vector2f size;
		};

		Args args;
		args.offset = offset;
		args.size   = size;

		ctx->bind_srv(src, m_scene->binding);
		ctx->bind_sampler(RHIBilinearSampler::static_sampler(), m_scene->binding);
		ctx->update_scalar(&args, m_args);
		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(BloomExtract, "[shaders]:/TrinexEngine/trinex/graphics/bloom/extract.slang")
	{
		m_scene = find_parameter("scene");
		m_args  = find_parameter("args");
	}

	void BloomExtract::extract(RHIContext* ctx, RHIShaderResourceView* src, float threshold, float knee, float clamp,
	                           Vector2f offset, Vector2f size)
	{
		push_context_state(this, ctx);

		struct Args {
			alignas(8) Vector2f offset;
			alignas(8) Vector2f size;
			alignas(4) float threshold;
			alignas(4) float knee;
			alignas(4) float clamp;
		};

		Args args;
		args.offset    = offset;
		args.size      = size;
		args.threshold = threshold;
		args.knee      = threshold * knee;
		args.clamp     = clamp;

		ctx->bind_srv(src, m_scene->binding);
		ctx->bind_sampler(RHIBilinearSampler::static_sampler(), m_scene->binding);
		ctx->update_scalar(&args, sizeof(args), m_args);
		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(BloomDownsample, "[shaders]:/TrinexEngine/trinex/graphics/bloom/downsample.slang")
	{
		m_scene = find_parameter("scene");
	}

	void BloomDownsample::downsample(RHIContext* ctx, RHIShaderResourceView* src)
	{
		push_context_state(this, ctx);

		ctx->bind_srv(src, m_scene->binding);
		ctx->bind_sampler(RHIBilinearSampler::static_sampler(), m_scene->binding);
		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(BloomUpsample, "[shaders]:/TrinexEngine/trinex/graphics/bloom/upsample.slang")
	{
		m_scene = find_parameter("scene");
		m_args  = find_parameter("args");
	}

	void BloomUpsample::upsample(RHIContext* ctx, RHIShaderResourceView* src, float weight, Vector2f offset, Vector2f size)
	{
		push_context_state(this, ctx);

		struct Args {
			Vector2f offset;
			Vector2f size;
			float weight;
		};

		Args args;
		args.offset = offset;
		args.size   = size;
		args.weight = weight;

		ctx->bind_srv(src, m_scene->binding);
		ctx->bind_sampler(RHIBilinearSampler::static_sampler(), m_scene->binding);
		ctx->update_scalar(&args, m_args);
		ctx->draw(6, 0);

		pop_context_state(ctx);
	}

	trinex_implement_pipeline(BatchedLines, "[shaders]:/TrinexEngine/trinex/graphics/batched_lines.slang")
	{
		m_projview = find_parameter("projview");
		m_viewport = find_parameter("viewport");
	}

	trinex_implement_pipeline(BatchedTriangles, "[shaders]:/TrinexEngine/trinex/graphics/batched_triangles.slang") {}

	trinex_implement_pipeline(DeferredLighting, "[shaders]:/TrinexEngine/trinex/lighting/deferred.slang")
	{
		scene_view         = find_parameter("scene_view");
		base_color_texture = find_parameter("base_color_texture");
		normal_texture     = find_parameter("normal_texture");
		msra_texture       = find_parameter("msra_texture");
		depth_texture      = find_parameter("depth_texture");

		screen_sampler = find_parameter("screen_sampler");
		shadow_sampler = find_parameter("shadow_sampler");

		ranges   = find_parameter("ranges");
		clusters = find_parameter("clusters");
		lights   = find_parameter("lights");
		shadows  = find_parameter("shadows");
	}

	trinex_implement_pipeline(AmbientLight, "[shaders]:/TrinexEngine/trinex/lighting/ambient.slang")
	{
		//setup_lighting_pipeline_state(this);

		scene_view    = find_parameter("scene_view");
		base_color    = find_parameter("base_color");
		msra          = find_parameter("msra");
		ambient_color = find_parameter("ambient_color");
	}

	trinex_implement_pipeline(TonemappingACES, "[shaders]:/TrinexEngine/trinex/graphics/tonemapping.slang")
	{
		m_hdr_target = find_parameter("hdr_scene");
		m_scene_view = find_parameter("scene_view");
	}

	TonemappingACES& TonemappingACES::apply(RHIContext* ctx, Renderer* renderer)
	{
		ctx->begin_rendering(renderer->scene_color_ldr_target()->as_rtv());
		{
			push_context_state(this, ctx);
			ctx->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
			ctx->bind_srv(renderer->scene_color_hdr_target()->as_srv(), m_hdr_target->binding);
			ctx->bind_sampler(RHIPointSampler::static_sampler(), m_hdr_target->binding);
			ctx->draw(6, 0);
			pop_context_state(ctx);
		}
		ctx->end_rendering();

		return *this;
	}

	trinex_implement_pipeline(SSR, "[shaders]:/TrinexEngine/trinex/graphics/ssr.slang")
	{
		scene_view   = find_parameter("scene_view");
		scene_color  = find_parameter("scene_color");
		scene_normal = find_parameter("scene_normal");
		scene_depth  = find_parameter("scene_depth");
		sampler      = find_parameter("sampler");
	}

	trinex_implement_pipeline(SSAO, "[shaders]:/TrinexEngine/trinex/graphics/ssao.slang")
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

		m_samples_count = count;

		if (m_samples_buffer)
			m_samples_buffer->release();

		StackByteAllocator::Mark mark;
		Vector3f* kernel = StackAllocator<Vector3f>::allocate(count);

		for (size_t i = 0; i < count; ++i)
		{
			Vector3f& sample = kernel[i];

			sample.x = Random::floating() * 2.0f - 1.f;
			sample.y = Random::floating() * 2.0f - 1.f;
			sample.z = Random::floating() * 2.0f - 1.f;

			sample = glm::normalize(sample);
			sample *= Random::floating();
			float factor = static_cast<float>(i) / static_cast<float>(count);
			sample *= glm::mix(0.1f, 1.0f, factor * factor);
		}

		m_samples_buffer = rhi->create_buffer(count * sizeof(Vector3f),
		                                      RHIBufferCreateFlags::StructuredBuffer | RHIBufferCreateFlags::ShaderResource);

		RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
		{
			ctx->barrier(m_samples_buffer, RHIAccess::TransferDst);
			ctx->update_buffer(m_samples_buffer, 0, count * sizeof(Vector3f), reinterpret_cast<const byte*>(kernel));
		}
		RHIContextPool::global_instance()->end_context(ctx);
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
		args.noise_scale       = renderer->scene_view().view_size() / Vector2u(4, 4);
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

	trinex_implement_pipeline(ClusterInitialize, "[shaders]:/TrinexEngine/trinex/cluster/initialize.slang")
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

	trinex_implement_pipeline(ClusterLightCulling, "[shaders]:/TrinexEngine/trinex/cluster/light_culling.slang")
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

	trinex_implement_pipeline(CameraVelocity, "[shaders]:/TrinexEngine/trinex/graphics/velocity.slang")
	{
		m_scene_view = find_parameter("scene_view");
	}

	CameraVelocity& CameraVelocity::render(RHIContext* ctx, Renderer* renderer)
	{
		push_context_state(this, ctx);

		ctx->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		ctx->draw(6, 0);

		pop_context_state(ctx);
		return *this;
	}
}// namespace Engine::Pipelines
