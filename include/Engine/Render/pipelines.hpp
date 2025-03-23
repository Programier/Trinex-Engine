#pragma once
#include <Graphics/pipeline.hpp>

namespace Engine
{
	struct RHI_ShaderResourceView;
	struct RHI_UnorderedAccessView;
	struct RHI_Sampler;
	class SceneRenderer;

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

		trinex_declare_graphics_pipeline(BatchedLines,
		private:
			const ShaderParameterInfo* m_globals;

		public:
			void apply(SceneRenderer* renderer);
		);

		trinex_declare_graphics_pipeline(BatchedTriangles);

		trinex_declare_graphics_pipeline(DeferredPointLightShadowed,
		public:
			const ShaderParameterInfo* globals            = nullptr;
			const ShaderParameterInfo* base_color_texture = nullptr;
			const ShaderParameterInfo* normal_texture     = nullptr;
			const ShaderParameterInfo* emissive_texture   = nullptr;
			const ShaderParameterInfo* msra_texture       = nullptr;
			const ShaderParameterInfo* depth_texture      = nullptr;
			const ShaderParameterInfo* color              = nullptr;
			const ShaderParameterInfo* intensivity        = nullptr;

			const ShaderParameterInfo* shadow_map_texture  = nullptr;
			const ShaderParameterInfo* shadow_map_projview = nullptr;
			const ShaderParameterInfo* depth_bias          = nullptr;
			const ShaderParameterInfo* slope_scale         = nullptr;

			const ShaderParameterInfo* location          = nullptr;
			const ShaderParameterInfo* radius            = nullptr;
			const ShaderParameterInfo* fall_off_exponent = nullptr;

			DeferredPointLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_graphics_pipeline(DeferredPointLight,
		public:
			const ShaderParameterInfo* globals            = nullptr;
			const ShaderParameterInfo* base_color_texture = nullptr;
			const ShaderParameterInfo* normal_texture     = nullptr;
			const ShaderParameterInfo* emissive_texture   = nullptr;
			const ShaderParameterInfo* msra_texture       = nullptr;
			const ShaderParameterInfo* depth_texture      = nullptr;
			const ShaderParameterInfo* color              = nullptr;
			const ShaderParameterInfo* intensivity        = nullptr;

			const ShaderParameterInfo* location          = nullptr;
			const ShaderParameterInfo* radius            = nullptr;
			const ShaderParameterInfo* fall_off_exponent = nullptr;

			DeferredPointLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_graphics_pipeline(DeferredSpotLightShadowed,
		public:
			const ShaderParameterInfo* globals            = nullptr;
			const ShaderParameterInfo* base_color_texture = nullptr;
			const ShaderParameterInfo* normal_texture     = nullptr;
			const ShaderParameterInfo* emissive_texture   = nullptr;
			const ShaderParameterInfo* msra_texture       = nullptr;
			const ShaderParameterInfo* depth_texture      = nullptr;
			const ShaderParameterInfo* color              = nullptr;
			const ShaderParameterInfo* intensivity        = nullptr;

			const ShaderParameterInfo* shadow_map_texture  = nullptr;
			const ShaderParameterInfo* shadow_map_projview = nullptr;
			const ShaderParameterInfo* depth_bias          = nullptr;
			const ShaderParameterInfo* slope_scale         = nullptr;

			const ShaderParameterInfo* location          = nullptr;
			const ShaderParameterInfo* radius            = nullptr;
			const ShaderParameterInfo* fall_off_exponent = nullptr;

			const ShaderParameterInfo* direction         = nullptr;
			const ShaderParameterInfo* spot_angles       = nullptr;

			DeferredSpotLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_graphics_pipeline(DeferredSpotLight,
		public:
			const ShaderParameterInfo* globals            = nullptr;
			const ShaderParameterInfo* base_color_texture = nullptr;
			const ShaderParameterInfo* normal_texture     = nullptr;
			const ShaderParameterInfo* emissive_texture   = nullptr;
			const ShaderParameterInfo* msra_texture       = nullptr;
			const ShaderParameterInfo* depth_texture      = nullptr;
			const ShaderParameterInfo* color              = nullptr;
			const ShaderParameterInfo* intensivity        = nullptr;

			const ShaderParameterInfo* location          = nullptr;
			const ShaderParameterInfo* radius            = nullptr;
			const ShaderParameterInfo* fall_off_exponent = nullptr;

			const ShaderParameterInfo* direction         = nullptr;
			const ShaderParameterInfo* spot_angles       = nullptr;

			DeferredSpotLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_graphics_pipeline(DeferredDirectionalLightShadowed,
		public:
			const ShaderParameterInfo* globals            = nullptr;
			const ShaderParameterInfo* base_color_texture = nullptr;
			const ShaderParameterInfo* normal_texture     = nullptr;
			const ShaderParameterInfo* emissive_texture   = nullptr;
			const ShaderParameterInfo* msra_texture       = nullptr;
			const ShaderParameterInfo* depth_texture      = nullptr;
			const ShaderParameterInfo* color              = nullptr;
			const ShaderParameterInfo* intensivity        = nullptr;

			const ShaderParameterInfo* shadow_map_texture  = nullptr;
			const ShaderParameterInfo* shadow_map_projview = nullptr;
			const ShaderParameterInfo* depth_bias          = nullptr;
			const ShaderParameterInfo* slope_scale         = nullptr;
			const ShaderParameterInfo* direction			= nullptr;

			DeferredDirectionalLightShadowed& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		trinex_declare_graphics_pipeline(DeferredDirectionalLight,
		public:
			const ShaderParameterInfo* globals            = nullptr;
			const ShaderParameterInfo* base_color_texture = nullptr;
			const ShaderParameterInfo* normal_texture     = nullptr;
			const ShaderParameterInfo* emissive_texture   = nullptr;
			const ShaderParameterInfo* msra_texture       = nullptr;
			const ShaderParameterInfo* depth_texture      = nullptr;
			const ShaderParameterInfo* color              = nullptr;
			const ShaderParameterInfo* intensivity        = nullptr;
			const ShaderParameterInfo* direction			= nullptr;

			DeferredDirectionalLight& modify_compilation_env(ShaderCompilationEnvironment* env) override;
		);

		// clang-format on
	}// namespace Pipelines
}// namespace Engine
