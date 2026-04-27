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

namespace Trinex
{
	static ScriptFunction script_actor_spawned;
	static ScriptFunction script_actor_start_play;
	static ScriptFunction script_actor_update;
	static ScriptFunction script_actor_stop_play;
	static ScriptFunction script_actor_despawned;

	Actor::Actor() : m_default_components_count(0), m_is_playing(false), m_is_visible(true) {}

	Actor::~Actor()
	{
		while (!m_components.empty())
		{
			ActorComponent* component = m_components.back();
			component->owner(nullptr);
		}
	}

	void Actor::scriptable_spawned()
	{
		ScriptObject(this).execute(script_actor_spawned);
	}

	void Actor::scriptable_start_play()
	{
		ScriptObject(this).execute(script_actor_start_play);
	}

	void Actor::scriptable_update(float dt)
	{
		ScriptObject(this).execute(script_actor_update, nullptr, dt);
	}

	void Actor::scriptable_stop_play()
	{
		ScriptObject(this).execute(script_actor_stop_play);
	}

	void Actor::scriptable_despawned()
	{
		ScriptObject(this).execute(script_actor_despawned);
	}

	Actor* Actor::new_instance(Refl::Class* self, const Vector3f& location, const Quaternion& rotation, const Vector3f& scale,
	                           const Name& name)
	{
		Actor* actor = self->create_object()->instance_cast<Actor>();

		if (actor == nullptr)
			return nullptr;

		if (SceneComponent* scene_component = actor->scene_component())
		{
			scene_component->local_transform(location, rotation, scale);
		}

		return actor;
	}

	Object* Actor::register_child(Object* child, u32& index)
	{
		ActorComponent* component = instance_cast<ActorComponent>(child);

		if (component == nullptr)
			return Super::register_child(child, index);

		index = m_components.size();
		m_components.push_back(component);

		if (m_root_component == nullptr)
		{
			m_root_component = instance_cast<SceneComponent>(component);
		}

		component->spawned();

		if (is_playing())
		{
			component->start_play();
		}

		return this;
	}

	bool Actor::unregister_child(Object* child)
	{
		ActorComponent* component = instance_cast<ActorComponent>(child);

		if (component == nullptr)
			return Super::unregister_child(child);

		if (component->is_playing())
		{
			component->stop_play();
		}

		component->despawned();

		if (m_root_component == component)
		{
			m_root_component = nullptr;
		}

		return component->remove_from(m_components);
	}

	bool Actor::is_visible() const
	{
		return m_is_visible && level()->is_visible();
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

	Actor& Actor::on_create()
	{
		Super::on_create();
		m_default_components_count = m_components.size();
		return *this;
	}

	Actor& Actor::spawned()
	{
		for (usize index = 0, count = m_components.size(); index < count; ++index)
		{
			auto component = m_components[index];
			component->spawned();
		}
		return *this;
	}

	Actor& Actor::start_play()
	{
		if (!is_playing())
		{
			m_is_playing = true;

			for (auto& component : m_components)
			{
				component->start_play();
			}
		}
		return *this;
	}

	Actor& Actor::update(float dt)
	{
		// Update each component in actor
		for (auto& component : m_components)
		{
			component->update(dt);
		}
		return *this;
	}

	Actor& Actor::stop_play()
	{
		if (m_is_playing)
		{
			m_is_playing = false;

			for (auto& component : m_components)
			{
				component->stop_play();
			}
		}

		return *this;
	}

	Actor& Actor::despawned()
	{
		if (is_playing())
		{
			stop_play();
		}

		for (usize index = 0, count = m_components.size(); index < count; ++index)
		{
			ActorComponent* component = m_components[index];
			component->despawned();
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

	LevelInstance* Actor::level() const
	{
		return instance_cast<LevelInstance>(owner());
	}

	World* Actor::world() const
	{
		if (LevelInstance* actor_level = level())
			return actor_level->world();
		return nullptr;
	}

	RenderScene* Actor::scene() const
	{
		if (World* actor_world = world())
			return actor_world->scene();
		return nullptr;
	}

	bool Actor::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;

		for (u32 i = 0; i < m_default_components_count; ++i)
		{
			u32 offset;
			ar.begin_chunk(offset);
			m_components[i]->serialize(ar);
			ar.end_chunk(offset);
		}

		return ar;
	}

	template<typename T>
	class ActorComponentsExt : public T
	{
	public:
		using T::T;

		const String& index_name(const void* context, usize index) const override
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

		script_actor_spawned    = r.method("void spawned()", trinex_scoped_void_method(Actor, spawned));
		script_actor_start_play = r.method("void start_play()", trinex_scoped_void_method(Actor, start_play));
		script_actor_update     = r.method("void update(float dt)", trinex_scoped_void_method(Actor, update));
		script_actor_stop_play  = r.method("void stop_play()", trinex_scoped_void_method(Actor, stop_play));
		script_actor_despawned  = r.method("void despawned()", trinex_scoped_void_method(Actor, despawned));

		r.method("SceneComponent@ scene_component() const final", &This::scene_component);

		ScriptEngine::on_terminate.push([]() {
			script_actor_spawned.release();
			script_actor_start_play.release();
			script_actor_update.release();
			script_actor_stop_play.release();
			script_actor_despawned.release();
		});

		auto flags      = Refl::Property::IsReadOnly | Refl::Property::IsTransient;
		auto components = trinex_refl_prop(m_components, flags);

		if (auto element = Refl::Object::instance_cast<Refl::ObjectProperty>(components->element_property()))
		{
			element->is_composite(true);
		}

		components->display_name("Components").tooltip("Array of components of this actor");
		trinex_refl_virtual_prop(Is Visible, is_visible, is_visible)
		        ->display_name("Is Visible")
		        .tooltip("If true, actor is visible in the scene");
	}
}// namespace Trinex
