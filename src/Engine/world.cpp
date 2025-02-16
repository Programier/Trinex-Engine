#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Systems/logic_system.hpp>

namespace Engine
{
	implement_engine_class_default_init(World, 0);

	World* World::current = nullptr;

	World& World::create()
	{
		Super::create();
		LogicSystem::new_system<LogicSystem>()->register_subsystem(this);
		m_is_playing = false;

		m_scene = new Scene();

		return *this;
	}

	World& World::wait()
	{
		Super::wait();
		return *this;
	}

	World& World::update(float dt)
	{
		Super::update(dt);

		if (!m_is_playing)
			return *this;

		current = this;

		if (!m_actors_to_destroy.empty())
		{
			while (!m_actors_to_destroy.empty())
			{
				auto& front = m_actors_to_destroy.front();

				if (front.skip_frames == 0)
				{
					destroy_actor(front.actor, true);
					m_actors_to_destroy.pop_front();
				}
				else
					break;
			}

			for (auto& actor_info : m_actors_to_destroy)
			{
				--actor_info.skip_frames;
			}
		}

		for (size_t index = 0, count = m_actors.size(); index < count; ++index)
		{
			Actor* actor = m_actors[index];
			if (actor->is_playing())
			{
				m_actors[index]->update(dt);
			}
		}

		current = nullptr;

		return *this;
	}

	World& World::destroy_all_actors()
	{
		for (auto& info : m_actors_to_destroy)
		{
			destroy_actor(info.actor, true);
		}

		m_actors_to_destroy.clear();

		while (!m_actors.empty())
		{
			destroy_actor(m_actors.front(), true);
		}

		return *this;
	}

	World& World::shutdown()
	{
		Super::shutdown();
		stop_play();
		destroy_all_actors();
		render_thread()->wait();
		delete m_scene;
		return *this;
	}

	World& World::start_play()
	{
		if (m_is_playing)
			return *this;

		for (size_t index = 0; index < m_actors.size(); index++)
		{
			m_actors[index]->start_play();
		}

		m_is_playing = true;
		return *this;
	}

	World& World::stop_play()
	{
		if (!m_is_playing)
			return *this;

		for (size_t index = 0; index < m_actors.size(); index++)
		{
			m_actors[index]->stop_play();
		}

		m_is_playing = false;
		return *this;
	}

	Actor* World::spawn_actor(class Refl::Class* self, const Vector3f& location, const Vector3f& rotation, const Vector3f& scale,
	                          const Name& actor_name)
	{
		if (!self)
			return nullptr;

		Actor* actor = self->create_object(actor_name, this)->instance_cast<Actor>();

		if (actor == nullptr)
		{
			throw EngineException("Invalid class for actor!");
		}

		actor->spawned();

		{
			SceneComponent* root = actor->scene_component();
			if (root)
			{
				root->location(location);
				root->rotation(rotation);
				root->scale(scale);
				m_scene->root_component()->attach(root);
			}
		}

		if (m_is_playing)
		{
			actor->start_play();
		}

		m_actors.push_back(actor);
		return actor;
	}

	World& World::destroy_actor(Actor* actor, bool ignore_playing)
	{
		if (!actor || actor->world() != this)
			return *this;

		unselect_actor(actor);

		if (!ignore_playing && actor->is_playing())
		{
			actor->stop_play();
			// Perhaps the method will be called before World::update, so we skip one frame and only then delete the actor
			DestroyActorInfo info;
			info.actor       = actor;
			info.skip_frames = 1;
			m_actors_to_destroy.push_back(info);
			return *this;
		}


		if (actor->is_playing())
			actor->stop_play();

		actor->destroyed();
		actor->owner(nullptr);

		for (size_t index = 0, count = m_actors.size(); index < count; ++index)
		{
			if (m_actors[index] == actor)
			{
				m_actors.erase(m_actors.begin() + index);
				break;
			}
		}

		return *this;
	}

	World& World::destroy_actor(Actor* actor)
	{
		return destroy_actor(actor, false);
	}

	Scene* World::scene() const
	{
		return m_scene;
	}

	World& World::select_actor(Actor* actor)
	{
		if (actor->world() == this && !is_selected(actor))
		{
			m_selected_actors.insert(actor);
			actor->actor_flags(Actor::Selected, true);
			on_actor_select(this, actor);
		}
		return *this;
	}

	World& World::unselect_actor(Actor* actor)
	{
		if (is_selected(actor))
		{
			m_selected_actors.erase(actor);
			actor->actor_flags(Actor::Selected, false);

			on_actor_unselect(this, actor);
		}
		return *this;
	}

	World& World::select_actors(const Vector<Actor*>& actors)
	{
		for (Actor* actor : actors)
		{
			select_actor(actor);
		}
		return *this;
	}

	World& World::unselect_actors(const Vector<Actor*>& actors)
	{
		for (Actor* actor : actors)
		{
			unselect_actor(actor);
		}
		return *this;
	}

	World& World::unselect_actors()
	{
		for (auto& selected : m_selected_actors)
		{
			selected->actor_flags(Actor::Selected, false);
			on_actor_unselect(this, selected);
		}
		m_selected_actors.clear();
		return *this;
	}

	const TreeSet<Actor*>& World::selected_actors() const
	{
		return m_selected_actors;
	}

	bool World::is_selected(Actor* actor) const
	{
		return actor->world() == this && actor->actor_flags.has_all(Actor::Selected);
	}

	const Vector<class Actor*>& World::actors() const
	{
		return m_actors;
	}

	World::~World()
	{
		if (!is_shutdowned())
		{
			shutdown();
		}
	}

	World* World::global()
	{
		static World* global_world = nullptr;
		if (global_world == nullptr)
		{
			System* system = Object::static_find_object_checked<System>("Engine::Systems::EngineSystem");
			if (!system)
				return nullptr;
			system = system->find_subsystem("LogicSystem::Global World");
			if (!system)
				return nullptr;

			global_world = system->instance_cast<World>();
		}

		return global_world;
	}
}// namespace Engine
