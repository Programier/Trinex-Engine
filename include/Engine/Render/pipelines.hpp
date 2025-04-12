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

			struct LightData {
				Vector4f param1;
				Vector4f param2;
				float param3;

				inline void color(const Vector3f& col) { param1.x = col.x; param1.y = col.y; param1.z = col.z; }
				inline void intensivity(float value) { param1.w = value; }
				inline void location(const Vector3f& loc) { param2.x = loc.x; param2.y = loc.y; param2.z = loc.z; }
				inline void radius(float value) { param2.w = value; }
				inline void fall_off_exponent(float value) { param3 = value; }
			};

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

			struct LightData {
				Vector4f param1;
				Vector4f param2;
				Vector4f param3;
				Vector2f param4;

				inline void color(const Vector3f& col) { param1.x = col.x; param1.y = col.y; param1.z = col.z; }
				inline void intensivity(float value) { param1.w = value; }
				inline void direction(const Vector3f& dir) { param2.x = dir.x; param2.y = dir.y; param2.z = dir.z; }
				inline void radius(float value) { param2.w = value; }
				inline void location(const Vector3f& loc) { param3.x = loc.x; param3.y = loc.y; param3.z = loc.z; }
				inline void fall_off_exponent(float value) { param3.w = value; }
				inline void spot_angles(const Vector2f& angles) { param4 = angles; }
			};

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

			struct LightData {
				Vector4f param1;
				Vector3f param2;

				inline void color(const Vector3f& col) { param1.x = col.x; param1.y = col.y; param1.z = col.z; }
				inline void intensivity(float value) { param1.w = value; }
				inline void direction(const Vector3f& dir) { param2 = dir; }
			};

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
