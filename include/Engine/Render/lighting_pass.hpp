#pragma once
#include <Engine/Render/render_pass.hpp>

namespace Engine
{
	class LightComponentProxy;
	class SpotLightComponent;
	class PointLightComponent;
	class DirectionalLightComponent;

	class ENGINE_EXPORT ShadowlessLightingPass : public RenderPass
	{
		trinex_render_pass(ShadowlessLightingPass, RenderPass);
	};

	class LightingPass : public RenderPass
	{
		trinex_render_pass(ShadowlessLightingPass, RenderPass);
	};

	class ENGINE_EXPORT DeferredLightingPass : public RenderPass
	{
		trinex_render_pass(DeferredLightingPass, RenderPass);

		ShadowlessLightingPass* m_shadowless_lighting_pass = nullptr;
		LightingPass* m_lighting_pass                      = nullptr;

		RenderPass* find_render_pass(LightComponentProxy* light);

	public:
		DeferredLightingPass& initialize() override;
		bool is_empty() const override;
		DeferredLightingPass& render(RenderViewport*) override;

		DeferredLightingPass& add_light(SpotLightComponent* spotlight);
		DeferredLightingPass& add_light(PointLightComponent* spotlight);
		DeferredLightingPass& add_light(DirectionalLightComponent* spotlight);
	};
}// namespace Engine
