#pragma once
#include <Core/etl/array.hpp>
#include <Engine/Render/renderer.hpp>

namespace Trinex
{
	namespace Pipelines
	{
		class DeferredLightPipeline;
	}

	class LightComponent;
	class PostProcessComponent;
	class PointLightComponent;
	class SpotLightComponent;
	class DirectionalLightComponent;
	class MaterialBindings;
	struct PostProcessParameters;
	struct LightRenderRanges;
	struct Swizzle;

	class trinex_api DeferredRenderer : public Renderer
	{
	private:
		RHIBuffer* m_clusters_buffer                 = nullptr;
		RHIBuffer* m_lights_buffer                   = nullptr;
		RHIBuffer* m_shadow_buffer                   = nullptr;
		LightRenderRanges* m_light_ranges            = nullptr;
		PostProcessParameters* m_post_process_params = nullptr;

		FrameArray<PrimitiveComponent*> m_visible_primitives;
		FrameArray<LightComponent*> m_visible_lights;
		FrameArray<PostProcessComponent*> m_visible_post_processes;

	private:
		DeferredRenderer& register_debug_lines();

		DeferredRenderer& register_shadow_light(PointLightComponent* light, u8* shadow_data);
		DeferredRenderer& register_shadow_light(SpotLightComponent* light, u8* shadow_data);
		DeferredRenderer& register_shadow_light(DirectionalLightComponent* light, u8* shadow_data);

		DeferredRenderer& register_lit_mode_passes();
		DeferredRenderer& wireframe_pass(RHIContext* ctx);
		DeferredRenderer& geometry_pass(RHIContext* ctx);
		DeferredRenderer& reflection_pass(RHIContext* ctx);
		DeferredRenderer& velocity_pass(RHIContext* ctx);
		DeferredRenderer& translucent_pass(RHIContext* ctx);
		DeferredRenderer& ambient_occlusion_pass(RHIContext* ctx);
		DeferredRenderer& global_illumination_pass(RHIContext* ctx);
		DeferredRenderer& deferred_lighting_pass(RHIContext* ctx);
		DeferredRenderer& temporal_antialiasing_pass(RHIContext* ctx);
		DeferredRenderer& bloom_pass(RHIContext* ctx);

		DeferredRenderer& copy_to_scene_color(RHIContext* ctx, RHITexture* src, const Swizzle& swizzle);
		DeferredRenderer& copy_depth_to_scene_color(RHIContext* ctx);
		DeferredRenderer& cull_lights(RHIContext* ctx);

	public:
		DeferredRenderer(const SceneView& view, ViewMode mode);
		trinex_non_copyable(DeferredRenderer);
		trinex_non_moveable(DeferredRenderer);

		RHIBuffer* clusters_buffer();
		RHIBuffer* lights_buffer();
		RHIBuffer* shadow_buffer();

		inline const FrameArray<PrimitiveComponent*>& visible_primitives() const { return m_visible_primitives; }
		inline const FrameArray<LightComponent*>& visible_lights() const { return m_visible_lights; }
		inline const FrameArray<PostProcessComponent*> visible_post_processes() const { return m_visible_post_processes; }
		inline const PostProcessParameters* post_process_parameters() const { return m_post_process_params; }
	};
}// namespace Trinex
