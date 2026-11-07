#pragma once
#include <Graphics/pipeline_library.hpp>

namespace Trinex
{
	class RHIShaderResourceView;
	class RHISampler;
	class RHIBuffer;
	class RHITexture;
	class RHIContext;
	class Renderer;
	struct LightRenderRanges;

	struct Swizzle : VectorNT<4, u8>
	{
		enum Enum : u8
		{
			R    = 0,
			G    = 1,
			B    = 2,
			A    = 3,
			Zero = 4,
			One  = 5,
		};

		inline Swizzle() : VectorNT<4, u8>(R, G, B, A) {}
		using VectorNT<4, u8>::VectorNT;
	};

	namespace Pipelines
	{
		class trinex_api GaussianBlur : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(GaussianBlur, GlobalPipelineLibrary);

			const RHIBinding* m_source;
			const RHIBinding* m_args;

		public:
			static void blur(RHIContext* ctx, RHIShaderResourceView* src, Vector2f direction, float sigma, float radius,
			                 Swizzle swizzle = {}, RHISampler* sampler = nullptr, Vector2f offset = {0.f, 0.f},
			                 Vector2f size = {1.f, 1.f});
		};

		class trinex_api NoiseApplication : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(NoiseApplication, GlobalPipelineLibrary);

			const RHIBinding* m_noise;
			const RHIBinding* m_args;

		public:
			static void noise(RHIContext* ctx, RHITexture* noise, f32 opacity, f32 scale, RHISampler* sampler = nullptr,
			                  Vector2f offset = {0.f, 0.f}, Vector2f size = {1.f, 1.f});

			static inline void noise(RHIContext* ctx, f32 opacity, f32 scale, Vector2f offset = {0.f, 0.f},
			                         Vector2f size = {1.f, 1.f})
			{
				noise(ctx, nullptr, opacity, scale, nullptr, offset, size);
			}
		};

		class trinex_api Blit2D : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(Blit2D, GlobalPipelineLibrary);

			const RHIBinding* m_source;
			const RHIBinding* m_args;

		public:
			static void blit(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset, Vector2f inv_size,
			                 Swizzle swizzle = {}, RHISampler* sampler = nullptr);
		};

		class trinex_api Passthrow : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(Passthrow, GlobalPipelineLibrary);

			const RHIBinding* m_source;
			const RHIBinding* m_args;

		public:
			struct Args
			{
				RHISampler* sampler    = nullptr;
				RHIRegion store_region = {};
				RHIRegion copy_region  = {};
				Vector4f color_scale   = {1.f, 1.f, 1.f, 1.f};
				Vector4f color_bias    = {0.f, 0.f, 0.f, 0.f};
				Swizzle swizzle        = {};
			};

			static void passthrow(RHIContext* ctx, RHIShaderResourceView* src);
			static void passthrow(RHIContext* ctx, RHIShaderResourceView* src, const Args& args);
		};

		class trinex_api Downsample : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(Downsample, GlobalPipelineLibrary);

			const RHIBinding* m_scene;
			const RHIBinding* m_args;

		public:
			void downsample(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset = {0.f, 0.f},
			                Vector2f size = {1.f, 1.f});
		};

		class trinex_api BloomExtract : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BloomExtract, GlobalPipelineLibrary);

			const RHIBinding* m_scene;
			const RHIBinding* m_args;

		public:
			void extract(RHIContext* ctx, RHIShaderResourceView* src, float threshold = 1.f, float knee = 0.5, float clamp = 3.f,
			             Vector2f offset = {0.f, 0.f}, Vector2f size = {1.f, 1.f});
		};

		class trinex_api BloomDownsample : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BloomDownsample, GlobalPipelineLibrary);

			const RHIBinding* m_scene;
			const RHIBinding* m_args;

		public:
			void downsample(RHIContext* ctx, RHIShaderResourceView* src);
		};

		class trinex_api BloomUpsample : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BloomUpsample, GlobalPipelineLibrary);

			const RHIBinding* m_scene;
			const RHIBinding* m_args;

		public:
			void upsample(RHIContext* ctx, RHIShaderResourceView* src, float weight = 1.f, Vector2f offset = {0.f, 0.f},
			              Vector2f size = {1.f, 1.f});
		};

		class trinex_api BatchedLines : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BatchedLines, GlobalPipelineLibrary);

		private:
			const RHIBinding* m_projview;
			const RHIBinding* m_viewport;

		public:
			inline const RHIBinding* projview() const { return m_projview; }
			inline const RHIBinding* viewport() const { return m_viewport; }
		};

		class trinex_api BatchedTriangles : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BatchedTriangles, GlobalPipelineLibrary);
		};

		class trinex_api DeferredLighting : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(DeferredLighting, GlobalPipelineLibrary);

		public:
			const RHIBinding* scene_view         = nullptr;
			const RHIBinding* base_color_texture = nullptr;
			const RHIBinding* normal_texture     = nullptr;
			const RHIBinding* msra_texture       = nullptr;
			const RHIBinding* depth_texture      = nullptr;

			const RHIBinding* screen_sampler = nullptr;
			const RHIBinding* shadow_sampler = nullptr;

			const RHIBinding* ranges   = nullptr;
			const RHIBinding* clusters = nullptr;
			const RHIBinding* lights   = nullptr;
			const RHIBinding* shadows  = nullptr;
		};

		class trinex_api AmbientLight : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(AmbientLight, GlobalPipelineLibrary);

		public:
			const RHIBinding* scene_view    = nullptr;
			const RHIBinding* base_color    = nullptr;
			const RHIBinding* msra          = nullptr;
			const RHIBinding* ambient_color = nullptr;
		};

		class TonemappingACES : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(TonemappingACES, GlobalPipelineLibrary);

		private:
			const RHIBinding* m_hdr_target = nullptr;
			const RHIBinding* m_scene_view = nullptr;

		public:
			TonemappingACES& apply(RHIContext* ctx, Renderer* renderer);
		};

		class SSR : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(SSR, GlobalPipelineLibrary);

		public:
			const RHIBinding* scene_view   = nullptr;
			const RHIBinding* scene_color  = nullptr;
			const RHIBinding* scene_normal = nullptr;
			const RHIBinding* scene_depth  = nullptr;
			const RHIBinding* sampler      = nullptr;
		};

		class SSAO : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(SSAO, GlobalPipelineLibrary);

		private:
			const RHIBinding* m_scene_view   = nullptr;
			const RHIBinding* m_scene_depth  = nullptr;
			const RHIBinding* m_scene_normal = nullptr;
			const RHIBinding* m_noise        = nullptr;
			const RHIBinding* m_sampler      = nullptr;
			const RHIBinding* m_args         = nullptr;
			const RHIBinding* m_samples      = nullptr;

			RHIBuffer* m_samples_buffer = nullptr;
			usize m_samples_count       = 0;

			SSAO& create_samples_buffer(usize count);

		public:
			SSAO& render(RHIContext* ctx, Renderer* renderer, float intensity, float bias, float power, float radius,
			             float fade_out_distance, float fade_out_radius, u32 samples);
		};

		class ClusterInitialize : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(ClusterInitialize, GlobalPipelineLibrary);

		private:
			const RHIBinding* m_scene_view;
			const RHIBinding* m_clusters;

		public:
			RHIBuffer* create_clusters_buffer();
			ClusterInitialize& build(RHIContext* ctx, RHIBuffer* clusters, Renderer* renderer);
		};

		class ClusterLightCulling : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(ClusterLightCulling, GlobalPipelineLibrary);

		private:
			const RHIBinding* m_scene_view;
			const RHIBinding* m_clusters;
			const RHIBinding* m_lights;
			const RHIBinding* m_ranges;

		public:
			ClusterLightCulling& cull(RHIContext* ctx, Renderer* renderer, RHIBuffer* clusters, RHIBuffer* lights,
			                          const LightRenderRanges& ranges);
		};

		class CameraVelocity : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(CameraVelocity, GlobalPipelineLibrary);

		private:
			const RHIBinding* m_scene_view;

		public:
			CameraVelocity& render(RHIContext* ctx, Renderer* renderer);
		};

		class DepthView : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(DepthView, GlobalPipelineLibrary);

		private:
			const RHIBinding* m_scene_view;
			const RHIBinding* m_scene_depth;

		public:
			DepthView& render(RHIContext* ctx, Renderer* renderer);
		};

		class TAA : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(TAA, GlobalPipelineLibrary);

		private:
			const RHIBinding* m_scene_color;
			const RHIBinding* m_prev_scene_color;

		public:
			TAA& render(RHIContext* ctx, Renderer* renderer);
		};
	} // namespace Pipelines
} // namespace Trinex
