#pragma once
#include <Core/enums.hpp>
#include <Core/name.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene_view.hpp>
#include <Graphics/shader_parameters.hpp>

namespace Engine
{
	class Scene;
	class SpotLightComponent;
	class StaticMeshComponent;
	class SpriteComponent;
	class PrimitiveComponent;
	class LightComponent;
	class LocalLightComponent;
	class PointLightComponent;
	class DirectionalLightComponent;
	class RenderSurface;
	class RenderViewport;

	class RenderPass;
	class ShadowPass;
	class ClearPass;
	class DepthPass;
	class GeometryPass;
	class AmbientLightingPass;
	class DeferredLightingPass;
	class PostProcessPass;
	class OverlayPass;

	struct ENGINE_EXPORT RenderStatistics final {
		size_t visible_objects;

		FORCE_INLINE RenderStatistics& reset()
		{
			visible_objects = 0;
			return *this;
		}
	};

	class ENGINE_EXPORT SceneRenderer
	{
	protected:
		class GlobalShaderParametersManager* m_global_shader_params;
		Vector<SceneView> m_scene_views;

		RenderPass* m_first_pass = nullptr;
		RenderPass* m_last_pass  = nullptr;
		ViewMode m_view_mode     = ViewMode::Lit;

		static SceneRenderer* static_scene_renderer_of(RenderPass* pass);
		void create_pass_internal(RenderPass* next, RenderPass* current);

	public:
		RenderStatistics statistics;
		Scene* scene;

		SceneRenderer();
		delete_copy_constructors(SceneRenderer);

		virtual SceneRenderer& initialize();
		virtual SceneRenderer& finalize();
		SceneRenderer& push_global_parameters(GlobalShaderParameters* parameters = nullptr);
		SceneRenderer& pop_global_parameters();
		const GlobalShaderParameters& global_parameters() const;
		const SceneRenderer& bind_global_parameters(BindingIndex index) const;
		const SceneView& scene_view() const;

		FORCE_INLINE ViewMode view_mode() const { return m_view_mode; }

		FORCE_INLINE RenderPass* first_pass() const { return m_first_pass; }

		FORCE_INLINE RenderPass* last_pass() const { return m_last_pass; }

		SceneRenderer& view_mode(ViewMode new_mode);

		template<typename T = RenderPass, typename... Args, typename = std::enable_if<std::is_base_of_v<RenderPass, T>>>
		inline T* create_pass(RenderPass* next = nullptr, Args&&... args)
		{
			if (SceneRenderer* renderer = static_scene_renderer_of(next))
			{
				if (renderer != this)
					return nullptr;
			}

			auto pass = new T(std::forward<Args>(args)...);
			create_pass_internal(next, pass);
			return pass;
		}

		bool destroy_pass(RenderPass* pass);

		virtual RenderSurface* output_surface() const;
		virtual SceneRenderer& render(const SceneView& view, class RenderViewport* viewport);

		// Rendering part
		template<typename ComponentType>
		FORCE_INLINE SceneRenderer& render_base_component(ComponentType* component)
		{
			return render_component(static_cast<typename ComponentType::Super*>(component));
		}

		virtual SceneRenderer& render_component(PrimitiveComponent* component);
		virtual SceneRenderer& render_component(StaticMeshComponent* component);
		virtual SceneRenderer& render_component(SpriteComponent* component);
		virtual SceneRenderer& render_component(LightComponent* component);
		virtual SceneRenderer& render_component(LocalLightComponent* component);
		virtual SceneRenderer& render_component(PointLightComponent* component);
		virtual SceneRenderer& render_component(SpotLightComponent* component);
		virtual SceneRenderer& render_component(DirectionalLightComponent* component);

		virtual ~SceneRenderer();
	};

	class DepthSceneRenderer : public SceneRenderer
	{
	public:
		DepthSceneRenderer& initialize() override;
	};

	class ENGINE_EXPORT ColorSceneRenderer : public SceneRenderer
	{
	protected:
		DepthSceneRenderer* m_depth_renderer = nullptr;

	private:
		ShadowPass* m_shadow_pass                      = nullptr;
		ClearPass* m_clear_pass                        = nullptr;
		GeometryPass* m_geometry_pass                  = nullptr;
		AmbientLightingPass* m_ambient_lighting_pass   = nullptr;
		DeferredLightingPass* m_deferred_lighting_pass = nullptr;
		PostProcessPass* m_post_process_pass           = nullptr;
		OverlayPass* m_overlay_pass                    = nullptr;

	public:
		ColorSceneRenderer& initialize() override;
		ColorSceneRenderer& finalize() override;
		virtual ColorSceneRenderer& initialize_subrenderers();
		virtual ColorSceneRenderer& finalize_subrenderers();

		FORCE_INLINE ShadowPass* shadow_pass() const { return m_shadow_pass; }
		FORCE_INLINE ClearPass* clear_pass() const { return m_clear_pass; }
		FORCE_INLINE AmbientLightingPass* ambient_lighting_pass() const { return m_ambient_lighting_pass; }
		FORCE_INLINE GeometryPass* geometry_pass() const { return m_geometry_pass; }
		FORCE_INLINE DeferredLightingPass* deferred_lighting_pass() const { return m_deferred_lighting_pass; }
		FORCE_INLINE PostProcessPass* post_process_pass() const { return m_post_process_pass; }
		FORCE_INLINE OverlayPass* overlay_pass() const { return m_overlay_pass; }

		// Components rendering
		using SceneRenderer::render_component;
		ColorSceneRenderer& render_component(PointLightComponent* component) override;
		ColorSceneRenderer& render_component(SpotLightComponent* component) override;
		ColorSceneRenderer& render_component(DirectionalLightComponent* component) override;
	};

}// namespace Engine
