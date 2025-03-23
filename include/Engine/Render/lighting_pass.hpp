#pragma once
#include <Engine/Render/render_pass.hpp>

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
		ShadowPass& add_light(DepthSceneRenderer* renderer, SpotLightComponent* component);
	};

	class ENGINE_EXPORT ShadowedLightingPass : public RenderPass
	{
		trinex_render_pass(ShadowedLightingPass, RenderPass);
	};

	class LightingPass : public RenderPass
	{
		trinex_render_pass(LightingPass, RenderPass);
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

		DeferredLightingPass& add_light(SpotLightComponent* spotlight);
		DeferredLightingPass& add_light(PointLightComponent* spotlight);
		DeferredLightingPass& add_light(DirectionalLightComponent* spotlight);
	};
}// namespace Engine
