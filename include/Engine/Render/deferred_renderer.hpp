#pragma once
#include <Core/etl/vector.hpp>
#include <Engine/Render/renderer.hpp>

namespace Engine
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

	class ENGINE_EXPORT DeferredRenderer : public Renderer
	{
	private:
		RHIBuffer* m_clusters_buffer                 = nullptr;
		RHIBuffer* m_lights_buffer                   = nullptr;
		RHIBuffer* m_shadow_buffer                   = nullptr;
		LightRenderRanges* m_light_ranges            = nullptr;
		PostProcessParameters* m_post_process_params = nullptr;

		FrameVector<PrimitiveComponent*> m_visible_primitives;
		FrameVector<LightComponent*> m_visible_lights;
		FrameVector<PostProcessComponent*> m_visible_post_processes;

	private:
		DeferredRenderer& register_debug_lines();

		DeferredRenderer& register_shadow_light(PointLightComponent* light, byte* shadow_data);
		DeferredRenderer& register_shadow_light(SpotLightComponent* light, byte* shadow_data);
		DeferredRenderer& register_shadow_light(DirectionalLightComponent* light, byte* shadow_data);

		DeferredRenderer& register_lit_mode_passes();
		DeferredRenderer& geometry_pass(RHIContext* ctx);
		DeferredRenderer& translucent_pass(RHIContext* ctx);
		DeferredRenderer& ambient_occlusion_pass(RHIContext* ctx);
		DeferredRenderer& global_illumination_pass(RHIContext* ctx);
		DeferredRenderer& deferred_lighting_pass(RHIContext* ctx);
		DeferredRenderer& bloom_pass(RHIContext* ctx);
		DeferredRenderer& copy_base_color_to_scene_color(RHIContext* ctx);
		DeferredRenderer& copy_world_normal_to_scene_color(RHIContext* ctx);
		DeferredRenderer& copy_metalic_to_scene_color(RHIContext* ctx);
		DeferredRenderer& copy_specular_to_scene_color(RHIContext* ctx);
		DeferredRenderer& copy_roughness_to_scene_color(RHIContext* ctx);
		DeferredRenderer& copy_emissive_to_scene_color(RHIContext* ctx);
		DeferredRenderer& copy_ambient_to_scene_color(RHIContext* ctx);
		DeferredRenderer& copy_world_to_scene_color(RHIContext* ctx);
		DeferredRenderer& render_visible_primitives(RHIContext* ctx, RenderPass* pass, MaterialBindings* bindings = nullptr);
		DeferredRenderer& cull_lights(RHIContext* ctx);

	public:
		DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode);
		trinex_non_copyable(DeferredRenderer);
		trinex_non_moveable(DeferredRenderer);

		RHIBuffer* clusters_buffer();
		RHIBuffer* lights_buffer();
		RHIBuffer* shadow_buffer();
		DeferredRenderer& render(RHIContext* cxt) override;

		inline const FrameVector<PrimitiveComponent*>& visible_primitives() const { return m_visible_primitives; }
		inline const FrameVector<LightComponent*>& visible_lights() const { return m_visible_lights; }
		inline const FrameVector<PostProcessComponent*> visible_post_processes() const { return m_visible_post_processes; }
		inline const PostProcessParameters* post_process_parameters() const { return m_post_process_params; }
	};
}// namespace Engine
