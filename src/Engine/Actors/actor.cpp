#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
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

	void Actor::scriptable_update(float dt)
	{
		ScriptObject(this).execute(script_actor_update, nullptr, dt);
	}

	void Actor::scriptable_start_play()
	{
		ScriptObject(this).execute(script_actor_start_play);
	}

	void Actor::scriptable_stop_play()
	{
		ScriptObject(this).execute(script_actor_stop_play);
	}

	void Actor::scriptable_spawned()
	{
		ScriptObject(this).execute(script_actor_spawned);
	}

	void Actor::scriptable_destroyed()
	{
		ScriptObject(this).execute(script_actor_destroyed);
	}

	ActorComponent* Actor::create_component(Refl::Class* self, const Name& component_name)
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
			auto component = m_owned_components[index];
			component->spawned();
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
			stop_play();
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
		return m_root_component ? m_root_component->world_transform() : default_value_of<Transform>();
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

	bool Actor::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return static_cast<bool>(archive);
	}

	template<typename T>
	class ActorComponentsExt : public T
	{
	public:
		using T::T;

		const String& index_name(const void* context, size_t index) const override
		{
			const Refl::ArrayProperty* prop = this;
			const ActorComponent* component = *prop->at_as<const ActorComponent*>(context, index);
			return component->name().to_string();
		}
	};

	trinex_implement_engine_class(Actor, Refl::Class::IsScriptable)
	{
		auto self = static_reflection();
		auto r    = ScriptClassRegistrar::existing_class(self);

		script_actor_update     = r.method("void update(float dt)", trinex_scoped_void_method(Actor, update));
		script_actor_start_play = r.method("void start_play()", trinex_scoped_void_method(Actor, start_play));
		script_actor_stop_play  = r.method("void stop_play()", trinex_scoped_void_method(Actor, stop_play));
		script_actor_spawned    = r.method("void spawned()", trinex_scoped_void_method(Actor, spawned));
		script_actor_destroyed  = r.method("void destroyed()", trinex_scoped_void_method(Actor, destroyed));

		constexpr ActorComponent* (*create_component)(Actor*, Refl::Class*, const Name&) =
		        [](Actor* actor, Refl::Class* self, const Name& name) { return actor->create_component(self, name); };
		r.method("ActorComponent create_component(Class self, const Name& name) final", create_component);

		ScriptEngine::on_terminate.push([]() {
			script_actor_update.release();
			script_actor_start_play.release();
			script_actor_stop_play.release();
			script_actor_spawned.release();
			script_actor_destroyed.release();
		});

		auto components = trinex_refl_prop_ext(ActorComponentsExt, m_owned_components, Refl::Property::IsReadOnly);

		if (auto element = Refl::Object::instance_cast<Refl::ObjectProperty>(components->element_property()))
		{
			element->is_composite(true);
		}

		components->display_name("Components").tooltip("Array of components of this actor");
		trinex_refl_prop(m_is_visible)->display_name("Is Visible").tooltip("If true, actor is visible in the scene");
	}
}// namespace Engine
