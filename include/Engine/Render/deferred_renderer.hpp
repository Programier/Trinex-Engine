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
	class SpotLightComponent;
	struct PostProcessParameters;
	struct LightRenderRanges;

	class ENGINE_EXPORT DeferredRenderer : public Renderer
	{
	private:
		RHIBuffer* m_clusters_buffer      = nullptr;
		RHIBuffer* m_lights_buffer        = nullptr;
		LightRenderRanges* m_light_ranges = nullptr;

		FrameVector<PrimitiveComponent*> m_visible_primitives;
		FrameVector<LightComponent*> m_visible_lights;
		FrameVector<PostProcessComponent*> m_visible_post_processes;
		FrameVector<RHITexture*> m_shadow_maps;
		FrameVector<Matrix4f> m_shadow_projections;


	private:
		DeferredRenderer& register_debug_lines();

		DeferredRenderer& register_shadow_light(SpotLightComponent* light, uint_t index);
		DeferredRenderer& register_lit_mode_passes();
		DeferredRenderer& geometry_pass();
		DeferredRenderer& ambient_occlusion_pass(PostProcessParameters* params);
		DeferredRenderer& deferred_lighting_pass();
		DeferredRenderer& copy_base_color_to_scene_color();
		DeferredRenderer& copy_world_normal_to_scene_color();
		DeferredRenderer& copy_metalic_to_scene_color();
		DeferredRenderer& copy_specular_to_scene_color();
		DeferredRenderer& copy_roughness_to_scene_color();
		DeferredRenderer& copy_ambient_to_scene_color();
		DeferredRenderer& copy_world_to_scene_color();
		DeferredRenderer& render_visible_primitives(RenderPass* pass);
		DeferredRenderer& cull_lights();

		uint_t find_light_range(uint_t light_type, uint_t& start, uint_t& end);

	public:
		DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode);
		trinex_non_copyable(DeferredRenderer);
		trinex_non_moveable(DeferredRenderer);

		RHIBuffer* clusters_buffer();
		RHIBuffer* lights_buffer();
		DeferredRenderer& render() override;

		inline const FrameVector<PrimitiveComponent*>& visible_primitives() const { return m_visible_primitives; }
		inline const FrameVector<LightComponent*>& visible_lights() const { return m_visible_lights; }
		inline const FrameVector<PostProcessComponent*> visible_post_processes() const { return m_visible_post_processes; }
	};
}// namespace Engine
