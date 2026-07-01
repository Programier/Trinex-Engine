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

	struct Swizzle : VectorNT<4, u8> {
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
		class ENGINE_EXPORT GaussianBlur : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(GaussianBlur, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_source;
			const RHIShaderParameterInfo* m_args;

		public:
			static void blur(RHIContext* ctx, RHIShaderResourceView* src, Vector2f direction, float sigma, float radius,
			                 Swizzle swizzle = {}, RHISampler* sampler = nullptr, Vector2f offset = {0.f, 0.f},
			                 Vector2f size = {1.f, 1.f});
		};

		class ENGINE_EXPORT NoiseApplication : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(NoiseApplication, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_noise;
			const RHIShaderParameterInfo* m_args;

		public:
			static void noise(RHIContext* ctx, RHITexture* noise, f32 opacity, f32 scale, RHISampler* sampler = nullptr,
			                  Vector2f offset = {0.f, 0.f}, Vector2f size = {1.f, 1.f});

			static inline void noise(RHIContext* ctx, f32 opacity, f32 scale, Vector2f offset = {0.f, 0.f},
			                         Vector2f size = {1.f, 1.f})
			{
				noise(ctx, nullptr, opacity, scale, nullptr, offset, size);
			}
		};

		class ENGINE_EXPORT Blit2D : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(Blit2D, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_source;
			const RHIShaderParameterInfo* m_args;

		public:
			static void blit(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset, Vector2f inv_size,
			                 Swizzle swizzle = {}, RHISampler* sampler = nullptr);
		};

		class ENGINE_EXPORT Passthrow : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(Passthrow, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_source;
			const RHIShaderParameterInfo* m_args;

		public:
			static void passthrow(RHIContext* ctx, RHIShaderResourceView* src, Swizzle swizzle = {}, RHIRegion region = {},
			                      RHISampler* sampler = nullptr);

			static void passthrow(RHIContext* ctx, RHIRegion viewport, RHIShaderResourceView* src, Swizzle swizzle = {},
			                      RHIRegion region = {}, RHISampler* sampler = nullptr);
		};

		class ENGINE_EXPORT Downsample : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(Downsample, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void downsample(RHIContext* ctx, RHIShaderResourceView* src, Vector2f offset = {0.f, 0.f},
			                Vector2f size = {1.f, 1.f});
		};

		class ENGINE_EXPORT BloomExtract : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BloomExtract, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void extract(RHIContext* ctx, RHIShaderResourceView* src, float threshold = 1.f, float knee = 0.5, float clamp = 3.f,
			             Vector2f offset = {0.f, 0.f}, Vector2f size = {1.f, 1.f});
		};

		class ENGINE_EXPORT BloomDownsample : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BloomDownsample, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void downsample(RHIContext* ctx, RHIShaderResourceView* src);
		};

		class ENGINE_EXPORT BloomUpsample : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BloomUpsample, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_scene;
			const RHIShaderParameterInfo* m_args;

		public:
			void upsample(RHIContext* ctx, RHIShaderResourceView* src, float weight = 1.f, Vector2f offset = {0.f, 0.f},
			              Vector2f size = {1.f, 1.f});
		};


		class ENGINE_EXPORT BatchedLines : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BatchedLines, GlobalPipelineLibrary);

		private:
			const RHIShaderParameterInfo* m_projview;
			const RHIShaderParameterInfo* m_viewport;

		public:
			inline const RHIShaderParameterInfo* projview() const { return m_projview; }
			inline const RHIShaderParameterInfo* viewport() const { return m_viewport; }
		};

		class ENGINE_EXPORT BatchedTriangles : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(BatchedTriangles, GlobalPipelineLibrary);
		};

		class ENGINE_EXPORT DeferredLighting : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(DeferredLighting, GlobalPipelineLibrary);

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

		class ENGINE_EXPORT AmbientLight : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(AmbientLight, GlobalPipelineLibrary);

		public:
			const RHIShaderParameterInfo* scene_view    = nullptr;
			const RHIShaderParameterInfo* base_color    = nullptr;
			const RHIShaderParameterInfo* msra          = nullptr;
			const RHIShaderParameterInfo* ambient_color = nullptr;
		};

		class TonemappingACES : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(TonemappingACES, GlobalPipelineLibrary);

		private:
			const RHIShaderParameterInfo* m_hdr_target = nullptr;
			const RHIShaderParameterInfo* m_scene_view = nullptr;

		public:
			TonemappingACES& apply(RHIContext* ctx, Renderer* renderer);
		};

		class SSR : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(SSR, GlobalPipelineLibrary);

		public:
			const RHIShaderParameterInfo* scene_view   = nullptr;
			const RHIShaderParameterInfo* scene_color  = nullptr;
			const RHIShaderParameterInfo* scene_normal = nullptr;
			const RHIShaderParameterInfo* scene_depth  = nullptr;
			const RHIShaderParameterInfo* sampler      = nullptr;
		};

		class SSAO : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(SSAO, GlobalPipelineLibrary);

		private:
			const RHIShaderParameterInfo* m_scene_view   = nullptr;
			const RHIShaderParameterInfo* m_scene_depth  = nullptr;
			const RHIShaderParameterInfo* m_scene_normal = nullptr;
			const RHIShaderParameterInfo* m_noise        = nullptr;
			const RHIShaderParameterInfo* m_sampler      = nullptr;
			const RHIShaderParameterInfo* m_args         = nullptr;
			const RHIShaderParameterInfo* m_samples      = nullptr;

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
			const RHIShaderParameterInfo* m_scene_view;
			const RHIShaderParameterInfo* m_clusters;

		public:
			RHIBuffer* create_clusters_buffer();
			ClusterInitialize& build(RHIContext* ctx, RHIBuffer* clusters, Renderer* renderer);
		};

		class ClusterLightCulling : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(ClusterLightCulling, GlobalPipelineLibrary);

		private:
			const RHIShaderParameterInfo* m_scene_view;
			const RHIShaderParameterInfo* m_clusters;
			const RHIShaderParameterInfo* m_lights;
			const RHIShaderParameterInfo* m_ranges;

		public:
			ClusterLightCulling& cull(RHIContext* ctx, Renderer* renderer, RHIBuffer* clusters, RHIBuffer* lights,
			                          const LightRenderRanges& ranges);
		};

		class CameraVelocity : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(CameraVelocity, GlobalPipelineLibrary);

		private:
			const RHIShaderParameterInfo* m_scene_view;

		public:
			CameraVelocity& render(RHIContext* ctx, Renderer* renderer);
		};

		class DepthView : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(DepthView, GlobalPipelineLibrary);

		private:
			const RHIShaderParameterInfo* m_scene_view;
			const RHIShaderParameterInfo* m_scene_depth;

		public:
			DepthView& render(RHIContext* ctx, Renderer* renderer);
		};

		class TAA : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(TAA, GlobalPipelineLibrary);

		private:
			const RHIShaderParameterInfo* m_scene_color;
			const RHIShaderParameterInfo* m_prev_scene_color;

		public:
			TAA& render(RHIContext* ctx, Renderer* renderer);
		};
	}// namespace Pipelines
}// namespace Trinex
