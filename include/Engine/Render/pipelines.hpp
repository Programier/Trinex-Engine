#pragma once
#include <Graphics/pipeline.hpp>

namespace Engine
{
	struct RHI_ShaderResourceView;
	struct RHI_UnorderedAccessView;
	struct RHI_Sampler;
	class Renderer;

	namespace Pipelines
	{
		// clang-format off
		trinex_declare_compute_pipeline(GaussianBlur,
		private:
			const ShaderParameterInfo* m_src;
			const ShaderParameterInfo* m_dst;
			const ShaderParameterInfo* m_sigma;
			const ShaderParameterInfo* m_kernel_size;
		public:

			void blur(RHI_ShaderResourceView* src, RHI_UnorderedAccessView* dst, const Vector2u& dst_size, int32_t kernel = 5, float sigma = 2.f, RHI_Sampler* sampler = nullptr);
		);

		trinex_declare_compute_pipeline(Blit2D,
		private:
			const ShaderParameterInfo* m_src;
			const ShaderParameterInfo* m_dst;
			const ShaderParameterInfo* m_args;

		public:
			void blit(RHI_ShaderResourceView* src, RHI_UnorderedAccessView* dst, const Rect2D& src_rect, const Rect2D& dst_rect,
					  uint level = 0, Swizzle swizzle = {});
		);

		trinex_declare_compute_pipeline(Blit2DGamma,
		private:
			const ShaderParameterInfo* m_src;
			const ShaderParameterInfo* m_dst;
			const ShaderParameterInfo* m_args;

		public:
			void blit(RHI_ShaderResourceView* src, RHI_UnorderedAccessView* dst, const Rect2D& src_rect, const Rect2D& dst_rect,
					  float gamma, uint level = 0, Swizzle swizzle = {});
			Blit2DGamma& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_graphics_pipeline(BatchedLines,
		private:
			const ShaderParameterInfo* m_globals;

		public:
			void apply(Renderer* renderer);
		);

		trinex_declare_graphics_pipeline(BatchedTriangles);

		class ENGINE_EXPORT DeferredLightPipeline : public GlobalGraphicsPipeline
		{
		public:
			using GlobalGraphicsPipeline::GlobalGraphicsPipeline;

			const ShaderParameterInfo* globals            = nullptr;
			const ShaderParameterInfo* base_color_texture = nullptr;
			const ShaderParameterInfo* normal_texture     = nullptr;
			const ShaderParameterInfo* emissive_texture   = nullptr;
			const ShaderParameterInfo* msra_texture       = nullptr;
			const ShaderParameterInfo* depth_texture      = nullptr;
			const ShaderParameterInfo* parameters         = nullptr;

            void initialize() override;
		};

		trinex_declare_global_pipeline(DeferredPointLightShadowed, DeferredLightPipeline,
		public:
			DeferredPointLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_global_pipeline(DeferredPointLight, DeferredLightPipeline,
		public:
			DeferredPointLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_global_pipeline(DeferredSpotLightShadowed, DeferredLightPipeline,
        public:
			DeferredSpotLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_global_pipeline(DeferredSpotLight, DeferredLightPipeline,
        public:
			DeferredSpotLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_global_pipeline(DeferredDirectionalLightShadowed, DeferredLightPipeline,
        public:
			DeferredDirectionalLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_global_pipeline(DeferredDirectionalLight, DeferredLightPipeline,
		public:
			DeferredDirectionalLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_graphics_pipeline(AmbientLight,
		public:
			const ShaderParameterInfo* globals       = nullptr;
			const ShaderParameterInfo* base_color    = nullptr;
			const ShaderParameterInfo* msra          = nullptr;
			const ShaderParameterInfo* ambient_color = nullptr;
		);
		// clang-format on
	}// namespace Pipelines
}// namespace Engine
