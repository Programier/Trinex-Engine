#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/math/math.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_object.hpp>

namespace Engine
{
	static ScriptFunction script_scene_comp_transform_changed;
	static size_t childs_count(const SceneComponent* component)
	{
		return component->childs().size();
	}

	static SceneComponent* child_at(const SceneComponent* component, size_t index)
	{
		auto& childs = component->childs();
		if (childs.size() <= index)
			return nullptr;
		return childs[index];
	}

	trinex_implement_engine_class(SceneComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_virtual_prop(Is Visible, is_visible, is_visible);

		auto& local = *trinex_refl_prop(m_local);
		local.display_name("Transform").tooltip("Local transform of this component");

		auto r = ScriptClassRegistrar::existing_class(static_reflection());

		r.method("SceneComponent@ attach(SceneComponent@ child) final", &This::attach);
		r.method("SceneComponent@ detach_from_parent() final", &This::detach_from_parent);
		r.method("bool is_attached_to(SceneComponent@ component) const final", &This::is_attached_to);
		r.method("SceneComponent@ parent() const final", &This::parent);

		r.method("uint64 childs_count() const final", childs_count);
		r.method("SceneComponent@ child_at(uint64 index) const final", child_at);

		r.method("void despawned()", trinex_scoped_void_method(This, despawned));

		r.method("const Transform& local_transform() const final", overload_of<const Transform&()>(&This::local_transform));
		r.method("const Transform& world_transform() const final", &This::world_transform);
		r.method("SceneComponent@ local_transform(const Transform&) final",
		         overload_of<SceneComponent&(const Transform&)>(&This::local_transform));
		r.method("SceneComponent@ add_local_transform(const Transform&) final", &This::add_local_transform);
		r.method("SceneComponent@ remove_local_transform(const Transform&) final", &This::remove_local_transform);
		r.method("SceneComponent@ location(const Vector3f& new_location) final", &This::location);
		r.method("SceneComponent@ rotation(const Quaternion& new_rotation) final",
		         overload_of<SceneComponent&()>(&This::rotation));
		r.method("SceneComponent@ scale(const Vector3f& new_scale) final", &This::scale);
		r.method("SceneComponent@ add_location(const Vector3f& delta) final", &This::add_location);
		r.method("SceneComponent@ add_rotation(const Quaternion& delta) final",
		         overload_of<SceneComponent&()>(&This::add_rotation));
		r.method("SceneComponent@ add_scale(const Vector3f& delta) final", &This::add_scale);

		script_scene_comp_transform_changed =
		        r.method("void on_transform_changed()", trinex_scoped_void_method(This, on_transform_changed));

		ScriptEngine::on_terminate.push([]() { script_scene_comp_transform_changed.release(); });
	}

	SceneComponent::SceneComponent() : m_is_transform_dirty(false), m_is_visible(true), m_is_transform_changed(false) {}

	void SceneComponent::script_on_transform_changed()
	{
		ScriptObject(this).execute(script_scene_comp_transform_changed);
	}

	SceneComponent& SceneComponent::attach(SceneComponent* child)
	{
		trinex_check(child != this, "Cannot attach a component to itself");
		trinex_check(child && !is_attached_to(child), "Setting up attachment would create a cycle");

		child->detach_from_parent();

		child->m_parent = this;
		m_childs.push_back(child);
		return *this;
	}

	SceneComponent& SceneComponent::detach_from_parent()
	{
		if (m_parent)
		{
			for (size_t index = 0, count = m_parent->m_childs.size(); index < count; ++index)
			{
				SceneComponent* component = m_parent->m_childs[index];
				if (component == this)
				{
					m_parent->m_childs.erase(m_parent->m_childs.begin() + index);
					break;
				}
			}

			m_parent = nullptr;
		}

		return *this;
	}

	bool SceneComponent::is_attached_to(SceneComponent* component) const
	{
		if (component != nullptr)
		{
			for (const SceneComponent* comp = parent(); comp != nullptr; comp = comp->parent())
			{
				if (comp == component)
				{
					return true;
				}
			}
		}

		return false;
	}

	SceneComponent& SceneComponent::on_transform_changed()
	{
		m_is_transform_dirty = true;

		if (!m_is_transform_changed)
		{
			m_is_transform_changed = true;

			logic_thread()->add_task(Task([this]() {
				m_prev_world           = world_transform();
				m_is_transform_changed = false;
			}));
		}

		for (SceneComponent* child : m_childs)
		{
			child->on_transform_changed();
		}

		return *this;
	}

	SceneComponent& SceneComponent::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (event.is_a(&m_local))
		{
			m_is_transform_dirty = true;
			on_transform_changed();
		}

		return *this;
	}

	SceneComponent& SceneComponent::despawned()
	{
		detach_from_parent();
		Super::despawned();

		return *this;
	}

	const Transform& SceneComponent::world_transform() const
	{
		if (m_is_transform_dirty)
		{
			if (SceneComponent* parent_component = parent())
			{
				m_world = parent_component->world_transform() + m_local;
			}
			else
			{
				m_world = local_transform();
			}
			m_is_transform_dirty = false;
		}

		return m_world;
	}

	bool SceneComponent::is_visible() const
	{
		if (m_is_visible)
		{
			if (auto owner_actor = actor())
				return owner_actor->is_visible();
			return true;
		}

		return false;
	}

	SceneComponent& SceneComponent::is_visible(bool visible)
	{
		m_is_visible = visible;
		return *this;
	}

	SceneComponent& SceneComponent::local_transform(const Transform& transform)
	{
		m_local = transform;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::local_transform(const Vector3f& location, const Quaternion& rotation, const Vector3f& scale)
	{
		m_local.location = location;
		m_local.rotation = rotation;
		m_local.scale    = scale;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_local_transform(const Transform& transform)
	{
		m_local += transform;
		on_transform_changed();

		return *this;
	}

	SceneComponent& SceneComponent::remove_local_transform(const Transform& transform)
	{
		m_local -= transform;
		on_transform_changed();

		return *this;
	}

	SceneComponent& SceneComponent::location(const Vector3f& new_location)
	{
		m_local.location = new_location;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::rotation(const Quaternion& new_rotation)
	{
		m_local.rotation = new_rotation;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::scale(const Vector3f& new_scale)
	{
		m_local.scale = new_scale;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_location(const Vector3f& delta)
	{
		m_local.location += delta;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_rotation(const Quaternion& delta)
	{
		m_local.rotation *= delta;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_scale(const Vector3f& delta)
	{
		m_local.scale *= delta;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::look_at(const Vector3f& location, const Vector3f& up)
	{
		m_local.rotation = Math::quat_look_at(Math::normalize(location - local_transform().location), up);
		on_transform_changed();
		return *this;
	}

}// namespace Engine
