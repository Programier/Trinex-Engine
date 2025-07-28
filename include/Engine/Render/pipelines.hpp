#pragma once
#include <Graphics/pipeline.hpp>

namespace Engine
{
	class RHIShaderResourceView;
	class RHISampler;
	class RHIBuffer;
	class Renderer;

	namespace Pipelines
	{
		class ENGINE_EXPORT GaussianBlur : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(GaussianBlur, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_source;
			const RHIShaderParameterInfo* m_args;

		public:
			void blur(RHIShaderResourceView* src, Vector2f offset, Vector2f inv_size, Vector2f direction, float sigma,
			          float radius, Swizzle swizzle = {}, RHISampler* sampler = nullptr);
		};

		class ENGINE_EXPORT Blit2D : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(Blit2D, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_source;
			const RHIShaderParameterInfo* m_args;

		public:
			void blit(RHIShaderResourceView* src, Vector2f offset, Vector2f inv_size, Swizzle swizzle = {},
			          RHISampler* sampler = nullptr);
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

		class ENGINE_EXPORT DeferredLightPipeline : public GlobalGraphicsPipeline
		{
		public:
			using GlobalGraphicsPipeline::GlobalGraphicsPipeline;

			const RHIShaderParameterInfo* scene_view         = nullptr;
			const RHIShaderParameterInfo* base_color_texture = nullptr;
			const RHIShaderParameterInfo* normal_texture     = nullptr;
			const RHIShaderParameterInfo* msra_texture       = nullptr;
			const RHIShaderParameterInfo* depth_texture      = nullptr;
			const RHIShaderParameterInfo* parameters         = nullptr;
			const RHIShaderParameterInfo* shadow_map         = nullptr;
			const RHIShaderParameterInfo* shadow_projview    = nullptr;

			void initialize() override;
		};

		class ENGINE_EXPORT DeferredPointLightShadowed : public DeferredLightPipeline
		{
			trinex_declare_pipeline(DeferredPointLightShadowed, DeferredLightPipeline);

		public:
			DeferredPointLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		};

		class ENGINE_EXPORT DeferredPointLight : public DeferredLightPipeline
		{
			trinex_declare_pipeline(DeferredPointLight, DeferredLightPipeline);

		public:
			DeferredPointLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		};

		class ENGINE_EXPORT DeferredSpotLightShadowed : public DeferredLightPipeline
		{
			trinex_declare_pipeline(DeferredSpotLightShadowed, DeferredLightPipeline);

		public:
			DeferredSpotLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		};

		class ENGINE_EXPORT DeferredSpotLight : public DeferredLightPipeline
		{
			trinex_declare_pipeline(DeferredSpotLight, DeferredLightPipeline);

		public:
			DeferredSpotLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		};

		class ENGINE_EXPORT DeferredDirectionalLightShadowed : public DeferredLightPipeline
		{
			trinex_declare_pipeline(DeferredDirectionalLightShadowed, DeferredLightPipeline);

		public:
			DeferredDirectionalLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		};

		class ENGINE_EXPORT DeferredDirectionalLight : public DeferredLightPipeline
		{
			trinex_declare_pipeline(DeferredDirectionalLight, DeferredLightPipeline);

		public:
			DeferredDirectionalLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
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
			TonemappingACES& apply(Renderer* renderer);
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
			SSAO& render(Renderer* renderer, float intensity, float bias, float power, float radius, float fade_out_distance,
			             float fade_out_radius, uint_t samples);
		};
	}// namespace Pipelines
}// namespace Engine
