#pragma once
#include <Graphics/pipeline.hpp>

namespace Engine
{
	struct RHIShaderResourceView;
	struct RHIUnorderedAccessView;
	struct RHISampler;
	class Renderer;

	namespace Pipelines
	{
		class ENGINE_EXPORT GaussianBlur : public GlobalComputePipeline
		{
			trinex_declare_pipeline(GaussianBlur, GlobalComputePipeline);

			const RHIShaderParameterInfo* m_src;
			const RHIShaderParameterInfo* m_dst;
			const RHIShaderParameterInfo* m_sigma;
			const RHIShaderParameterInfo* m_kernel_size;

		public:
			void blur(RHIShaderResourceView* src, RHIUnorderedAccessView* dst, const Vector2u& dst_size, int32_t kernel = 5,
			          float sigma = 2.f, RHISampler* sampler = nullptr);
		};

		class ENGINE_EXPORT Blit2D : public GlobalComputePipeline
		{
			trinex_declare_pipeline(Blit2D, GlobalComputePipeline);

			const RHIShaderParameterInfo* m_src;
			const RHIShaderParameterInfo* m_dst;
			const RHIShaderParameterInfo* m_args;

		public:
			void blit(RHIShaderResourceView* src, RHIUnorderedAccessView* dst, const RHIRect& src_rect, const RHIRect& dst_rect,
			          uint_t level = 0, Swizzle swizzle = {});
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
			const RHIShaderParameterInfo* m_exposure   = nullptr;
			const RHIShaderParameterInfo* m_scene_view = nullptr;

		public:
			TonemappingACES& apply(Renderer* renderer);
		};
	}// namespace Pipelines
}// namespace Engine
