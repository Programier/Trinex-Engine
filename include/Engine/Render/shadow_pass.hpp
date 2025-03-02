#include <Engine/Render/render_pass.hpp>

namespace Engine
{
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
}// namespace Engine
