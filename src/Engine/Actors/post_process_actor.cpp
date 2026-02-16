#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/post_process_component.hpp>
#include <Engine/Actors/post_process_actor.hpp>

namespace Engine
{
	trinex_implement_engine_class(PostProcessActor, 0) {}

	PostProcessActor::PostProcessActor()
	{
		m_post_process_component = new_instance<PostProcessComponent>("Post Process Component", this);
	}
}// namespace Engine
