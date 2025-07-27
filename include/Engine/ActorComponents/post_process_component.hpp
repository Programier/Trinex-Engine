#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Render/post_process_parameters.hpp>

namespace Engine
{
	class ENGINE_EXPORT PostProcessComponent : public SceneComponent
	{
		trinex_declare_class(PostProcessComponent, SceneComponent);

	private:
		PostProcessParameters m_parameters;

		float m_priority;
		float m_blend_weight;

	public:
		PostProcessComponent();
		PostProcessComponent& start_play() override;
		PostProcessComponent& stop_play() override;

		inline const PostProcessParameters& parameters() const { return m_parameters; }
		inline float priority() const { return m_priority; }
		inline float blend_weight() const { return m_blend_weight; }
	};
}// namespace Engine
