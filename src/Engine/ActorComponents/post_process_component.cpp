#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/post_process_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>

namespace Engine
{
	trinex_implement_engine_class(PostProcessComponent, 0)
	{
		trinex_refl_prop(m_priority);
		trinex_refl_prop(m_blend_weight);
	}

	PostProcessComponent::PostProcessComponent() : m_priority(0.f), m_blend_weight(1.f) {}

	PostProcessComponent& PostProcessComponent::start_play()
	{
		Super::start_play();
		if (Actor* owner_actor = actor())
		{
			if (World* world = owner_actor->world())
			{
				if (Scene* scene = world->scene())
				{
					scene->add_post_process(this);
				}
			}
		}
		return *this;
	}

	PostProcessComponent& PostProcessComponent::stop_play()
	{
		Super::stop_play();

		if (Actor* owner_actor = actor())
		{
			if (World* world = owner_actor->world())
			{
				if (Scene* scene = world->scene())
				{
					scene->remove_post_process(this);
				}
			}
		}

		return *this;
	}
}// namespace Engine
