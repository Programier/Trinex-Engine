#pragma once
#include <Engine/Render/render_pass.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	class LightComponentProxy;
	class SpotLightComponent;
	class PointLightComponent;
	class DirectionalLightComponent;

	class SpotLightComponent;
	class DepthSceneRenderer;

	class ENGINE_EXPORT ShadowPass : public RenderPass
	{
		trinex_render_pass(ShadowPass, RenderPass);

	public:
		bool is_empty() const override;
		ShadowPass& render(RenderViewport* vp) override;
		ShadowPass& add_light(DepthSceneRenderer* renderer, SpotLightComponent* light);
		ShadowPass& add_light(DepthSceneRenderer* renderer, PointLightComponent* light);
		ShadowPass& add_light(DepthSceneRenderer* renderer, DirectionalLightComponent* light);
	};

	class ENGINE_EXPORT ShadowedLightingPass : public RenderPass
	{
		trinex_render_pass(ShadowedLightingPass, RenderPass);

	public:
		ShadowedLightingPass& add_deferred_light(SpotLightComponent* spotlight);
		ShadowedLightingPass& add_deferred_light(PointLightComponent* spotlight);
		ShadowedLightingPass& add_deferred_light(DirectionalLightComponent* spotlight);
	};

	class AmbientLightingPass : public RenderPass
	{
		trinex_render_pass(AmbientLightingPass, RenderPass);

	public:
		bool is_empty() const override;
		AmbientLightingPass& render(RenderViewport* vp) override;
	};

	class LightingPass : public RenderPass
	{
		trinex_render_pass(LightingPass, RenderPass);

		VertexBufferBase m_spot_lights;
		VertexBufferBase m_point_lights;
		VertexBufferBase m_directional_lights;

		uint32_t m_spot_light_count        = 0;
		uint32_t m_point_light_count       = 0;
		uint32_t m_directional_light_count = 0;

	public:
		LightingPass& initialize() override;
		LightingPass& clear() override;
		bool is_empty() const override;
		LightingPass& add_deferred_light(SpotLightComponent* light);
		LightingPass& add_deferred_light(PointLightComponent* light);
		LightingPass& add_deferred_light(DirectionalLightComponent* light);
		LightingPass& render(RenderViewport* vp) override;
	};

	class ENGINE_EXPORT DeferredLightingPass : public RenderPass
	{
		trinex_render_pass(DeferredLightingPass, RenderPass);

		ShadowedLightingPass* m_shadowed_lighting_pass = nullptr;
		LightingPass* m_lighting_pass                  = nullptr;

	public:
		DeferredLightingPass& initialize() override;
		bool is_empty() const override;
		DeferredLightingPass& render(RenderViewport*) override;

		DeferredLightingPass& add_light(SpotLightComponent* light);
		DeferredLightingPass& add_light(PointLightComponent* light);
		DeferredLightingPass& add_light(DirectionalLightComponent* light);
	};
}// namespace Engine
