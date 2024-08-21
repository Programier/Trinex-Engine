#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/property.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/world.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>

namespace Engine
{
	static ScriptFunction script_actor_update;
	static ScriptFunction script_actor_start_play;
	static ScriptFunction script_actor_stop_play;
	static ScriptFunction script_actor_spawned;
	static ScriptFunction script_actor_destroyed;


	const ScriptFunction& Actor::script_update_func()
	{
		return script_actor_update;
	}

	const ScriptFunction& Actor::script_start_play_func()
	{
		return script_actor_start_play;
	}

	const ScriptFunction& Actor::script_stop_play_func()
	{
		return script_actor_stop_play;
	}

	const ScriptFunction& Actor::script_spawned_func()
	{
		return script_actor_spawned;
	}

	const ScriptFunction& Actor::script_destroyed_func()
	{
		return script_actor_destroyed;
	}

	ActorComponent* Actor::create_component(Class* self, const Name& component_name)
	{
		if (self == nullptr)
			return nullptr;

		ActorComponent* component_object = self->create_object(component_name)->instance_cast<ActorComponent>();
		if (!component_object)
			throw EngineException("Cannot create actor component from non component class!");

		add_component(component_object);
		return component_object;
	}

	Actor& Actor::add_component(ActorComponent* component)
	{
		{
			Actor* owner = component->actor();
			if (owner)
			{
				owner->remove_component(component);
			}
		}

		component->owner(this);

		{
			SceneComponent* scene_component = component->instance_cast<SceneComponent>();
			if (!m_root_component && scene_component)
			{
				m_root_component = scene_component;
			}
			else if (m_root_component && scene_component)
			{
				m_root_component->attach(scene_component);
			}
		}

		m_owned_components.push_back(component);
		return *this;
	}

	Actor& Actor::remove_component(ActorComponent* component)
	{
		for (size_t i = 0, count = m_owned_components.size(); i < count; i++)
		{
			ActorComponent* actor_component = m_owned_components[i];
			if (actor_component == component)
			{
				if (m_root_component == component)
				{
					m_root_component = nullptr;
				}


				component->owner(nullptr);
				m_owned_components.erase(m_owned_components.begin() + i);
				break;
			}
		}
		return *this;
	}

	Actor& Actor::update(float dt)
	{
		// Update each component in actor
		for (auto& component : m_owned_components)
		{
			component->update(dt);
		}
		return *this;
	}

	Actor& Actor::start_play()
	{
		if (m_is_playing == false)
		{
			m_is_playing = true;

			for (auto& component : m_owned_components)
			{
				component->start_play();
			}
		}
		return *this;
	}

	Actor& Actor::stop_play()
	{
		if (m_is_playing)
		{
			m_is_playing = false;

			for (auto& component : m_owned_components)
			{
				component->stop_play();
			}
		}

		return *this;
	}

	bool Actor::is_visible() const
	{
		return m_is_visible;
	}

	Actor& Actor::is_visible(bool visible)
	{
		m_is_visible = visible;
		return *this;
	}

	bool Actor::is_playing() const
	{
		return m_is_playing;
	}

	bool Actor::is_selected() const
	{
		return actor_flags(Flag::Selected);
	}

	Actor& Actor::spawned()
	{
		for (Index index = 0, count = m_owned_components.size(); index < count; ++index)
		{
			m_owned_components[index]->spawned();
		}
		return *this;
	}

	Actor& Actor::destroy()
	{
		if (!m_is_being_destroyed)
		{
			world()->destroy_actor(this);
			m_is_being_destroyed = true;
		}
		return *this;
	}

	Actor& Actor::destroyed()
	{
		if (m_is_playing)
		{
			if (class_instance()->is_native())
			{
				stop_play();
			}
			else
			{
				ScriptObject object(this);
				object.execute(Actor::script_stop_play_func());
			}
		}

		// Call destroy for each component
		for (size_t index = 0, count = m_owned_components.size(); index < count; ++index)
		{
			ActorComponent* component = m_owned_components[index];
			component->destroyed();
			component->owner(nullptr);
		}

		return *this;
	}

	const Transform& Actor::transfrom() const
	{
		return m_root_component ? m_root_component->world_transform() : Transform::transform_zero;
	}

	SceneComponent* Actor::scene_component() const
	{
		return m_root_component.ptr();
	}

	const Vector<class ActorComponent*>& Actor::owned_components() const
	{
		return m_owned_components;
	}

	class World* Actor::world() const
	{
		return instance_cast<World>(owner());
	}

	class Scene* Actor::scene() const
	{
		if (World* actor_world = world())
		{
			return actor_world->scene();
		}
		return nullptr;
	}

	bool Actor::archive_process(Archive& archive)
	{
		if (!Super::archive_process(archive))
			return false;
		return static_cast<bool>(archive);
	}

	static void bind_to_scripts(ScriptClassRegistrar* registrar, Class*)
	{
		void (*update)(Actor*, float) = [](Actor* self, float dt) -> void { self->update(dt); };
		void (*start_play)(Actor*)    = [](Actor* self) -> void { self->start_play(); };
		void (*stop_play)(Actor*)     = [](Actor* self) -> void { self->stop_play(); };
		void (*spawned)(Actor*)       = [](Actor* self) -> void { self->spawned(); };
		void (*destroyed)(Actor*)     = [](Actor* self) -> void { self->destroyed(); };

		script_actor_update     = registrar->method("void update(float dt)", update);
		script_actor_start_play = registrar->method("void start_play()", start_play);
		script_actor_stop_play  = registrar->method("void stop_play()", stop_play);
		script_actor_spawned    = registrar->method("void spawned()", spawned);
		script_actor_destroyed  = registrar->method("void destroyed()", destroyed);
	}

	static void on_destroy()
	{
		script_actor_update.release();
		script_actor_start_play.release();
		script_actor_stop_play.release();
		script_actor_spawned.release();
		script_actor_destroyed.release();
	}

	implement_engine_class(Actor, Class::IsScriptable)
	{
		Class* self     = This::static_class_instance();
		auto components = new ArrayProperty("Components", "Array of components of this actor", &This::m_owned_components,
		                                    new ObjectProperty<This, ActorComponent>("", "", nullptr, Name::none), Name::none,
		                                    Property::Flag::IsConst);
		components->element_name_callback(default_array_object_element_name);
		self->add_property(components);
		self->add_property(new BoolProperty("Is Visible", "If true, actor is visible in the scene", &This::m_is_visible));
		self->script_registration_callback = bind_to_scripts;
		ScriptEngine::on_terminate.push(on_destroy);
	}


}// namespace Engine
