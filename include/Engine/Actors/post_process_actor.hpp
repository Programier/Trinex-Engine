#pragma once
#include <Engine/Actors/actor.hpp>

namespace Engine
{
	class PostProcessComponent;

	class ENGINE_EXPORT PostProcessActor : public Actor
	{
		trinex_declare_class(PostProcessActor, Actor);

	private:
		PostProcessComponent* m_post_process_component;

	public:
		PostProcessActor();
		inline PostProcessComponent* post_process_component() const { return m_post_process_component; }
	};
}// namespace Engine
