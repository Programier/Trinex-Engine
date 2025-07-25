#pragma once
#include <Engine/ActorComponents/scene_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT PostProcessComponent : public SceneComponent
	{
		trinex_declare_class(PostProcessComponent, SceneComponent);

	private:
		float m_priority;
		float m_blend_weight;

	public:
		PostProcessComponent();
		PostProcessComponent& start_play() override;
		PostProcessComponent& stop_play() override;
	};
}// namespace Engine
