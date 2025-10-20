#pragma once
#include <Graphics/pipeline.hpp>

namespace Engine
{
	class RHIShaderResourceView;
	class RHISampler;
	class RHIBuffer;
	class RHIContext;
	class Renderer;
	struct LightRenderRanges;

	namespace Pipelines
	{
		class ENGINE_EXPORT GaussianBlur : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(GaussianBlur, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_source;
			const RHIShaderParameterInfo* m_args;

		public:
			void blur(RHIContext* ctx, RHIShaderResourceView* src, Vector2f direction, float sigma, float radius,
			          Swizzle swizzle = {}, RHISampler* sampler = nullptr, Vector2f offset = {0.f, 0.f},
			          Vector2f size = {1.f, 1.f});
		};

		class ENGINE_EXPORT Blit2D : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(Blit2D, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_source;
			const RHIShaderParameterInfo* m_args;

		public:
			void blit(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset, Vector2f inv_size, Swizzle swizzle = {},
			          RHISampler* sampler = nullptr);
		};

		class ENGINE_EXPORT Passthrow : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(Passthrow, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void passthrow(RHIContext* ctx, RHIShaderResourceView* src, Vector4f color_offset = Vector4f(0.f),
			               Vector4f color_scale = Vector4f(1.f), Vector2f offset = {0.f, 0.f}, Vector2f size = {1.f, 1.f});
		};

		class ENGINE_EXPORT Downsample : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(Downsample, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void downsample(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset = {0.f, 0.f},
			                Vector2f size = {1.f, 1.f});
		};

		class ENGINE_EXPORT BloomExtract : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(BloomExtract, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void extract(RHIContext* ctx, RHIShaderResourceView* src, float threshold = 1.f, float knee = 0.5, float clamp = 3.f,
			             Vector2f offset = {0.f, 0.f}, Vector2f size = {1.f, 1.f});
		};

		class ENGINE_EXPORT BloomDownsample : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(BloomDownsample, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void downsample(RHIContext* ctx, RHIShaderResourceView* src);
		};

		class ENGINE_EXPORT BloomUpsample : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(BloomUpsample, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void upsample(RHIContext* ctx, RHIShaderResourceView* src, float weight = 1.f, Vector2f offset = {0.f, 0.f},
			              Vector2f size = {1.f, 1.f});
		};


		class ENGINE_EXPORT BatchedLines : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(BatchedLines, GlobalGraphicsPipeline);

		private:
			const RHIShaderParameterInfo* m_projview;
			const RHIShaderParameterInfo* m_viewport;

		public:
			inline const RHIShaderParameterInfo* projview() const { return m_projview; }
			inline const RHIShaderParameterInfo* viewport() const { return m_viewport; }
		};

		class ENGINE_EXPORT BatchedTriangles : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(BatchedTriangles, GlobalGraphicsPipeline);
		};

		class ENGINE_EXPORT DeferredLighting : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(DeferredLighting, GlobalGraphicsPipeline);

		public:
			const RHIShaderParameterInfo* scene_view         = nullptr;
			const RHIShaderParameterInfo* base_color_texture = nullptr;
			const RHIShaderParameterInfo* normal_texture     = nullptr;
			const RHIShaderParameterInfo* msra_texture       = nullptr;
			const RHIShaderParameterInfo* depth_texture      = nullptr;

			const RHIShaderParameterInfo* screen_sampler = nullptr;
			const RHIShaderParameterInfo* shadow_sampler = nullptr;

			const RHIShaderParameterInfo* ranges   = nullptr;
			const RHIShaderParameterInfo* clusters = nullptr;
			const RHIShaderParameterInfo* lights   = nullptr;
			const RHIShaderParameterInfo* shadows  = nullptr;
		};

		class ENGINE_EXPORT AmbientLight : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(AmbientLight, GlobalGraphicsPipeline);

		public:
			const RHIShaderParameterInfo* scene_view    = nullptr;
			const RHIShaderParameterInfo* base_color    = nullptr;
			const RHIShaderParameterInfo* msra          = nullptr;
			const RHIShaderParameterInfo* ambient_color = nullptr;
		};

		class TonemappingACES : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(TonemappingACES, GlobalGraphicsPipeline);

		private:
			const RHIShaderParameterInfo* m_hdr_target = nullptr;
			const RHIShaderParameterInfo* m_scene_view = nullptr;

		public:
			TonemappingACES& apply(RHIContext* ctx, Renderer* renderer);
		};

		class SSR : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(SSR, GlobalGraphicsPipeline);

		public:
			const RHIShaderParameterInfo* scene_view   = nullptr;
			const RHIShaderParameterInfo* scene_color  = nullptr;
			const RHIShaderParameterInfo* scene_normal = nullptr;
			const RHIShaderParameterInfo* scene_depth  = nullptr;
			const RHIShaderParameterInfo* sampler      = nullptr;
		};

		class SSAO : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(SSAO, GlobalGraphicsPipeline);

		private:
			const RHIShaderParameterInfo* m_scene_view   = nullptr;
			const RHIShaderParameterInfo* m_scene_depth  = nullptr;
			const RHIShaderParameterInfo* m_scene_normal = nullptr;
			const RHIShaderParameterInfo* m_noise        = nullptr;
			const RHIShaderParameterInfo* m_sampler      = nullptr;
			const RHIShaderParameterInfo* m_args         = nullptr;
			const RHIShaderParameterInfo* m_samples      = nullptr;

			RHIBuffer* m_samples_buffer = nullptr;
			size_t m_samples_count      = 0;

			SSAO& create_samples_buffer(size_t count);

		public:
			SSAO& render(RHIContext* ctx, Renderer* renderer, float intensity, float bias, float power, float radius,
			             float fade_out_distance, float fade_out_radius, uint_t samples);
		};

		class ClusterInitialize : public GlobalComputePipeline
		{
			trinex_declare_pipeline(ClusterInitialize, GlobalComputePipeline);

		private:
			const RHIShaderParameterInfo* m_scene_view;
			const RHIShaderParameterInfo* m_clusters;

		public:
			RHIBuffer* create_clusters_buffer();
			ClusterInitialize& build(RHIContext* ctx, RHIBuffer* clusters, Renderer* renderer);
		};

		class ClusterLightCulling : public GlobalComputePipeline
		{
			trinex_declare_pipeline(ClusterLightCulling, GlobalComputePipeline);

		private:
			const RHIShaderParameterInfo* m_scene_view;
			const RHIShaderParameterInfo* m_clusters;
			const RHIShaderParameterInfo* m_lights;
			const RHIShaderParameterInfo* m_ranges;

		public:
			ClusterLightCulling& cull(RHIContext* ctx, Renderer* renderer, RHIBuffer* clusters, RHIBuffer* lights,
			                          const LightRenderRanges& ranges);
		};
	}// namespace Pipelines
}// namespace Engine
