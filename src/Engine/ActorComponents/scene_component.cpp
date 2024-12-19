#include <Core/exception.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/scene_component.hpp>

namespace Engine
{
	implement_engine_class(SceneComponent, 0)
	{
		auto* self  = static_class_instance();
		auto& local = *trinex_refl_prop(self, This, m_local);
		local.display_name("Transform").tooltip("Local transform of this component");

		local.add_change_listener([](const Refl::PropertyChangedEvent& event) {
			SceneComponent* component = reinterpret_cast<SceneComponent*>(event.context);
			component->m_is_dirty     = true;
			component->on_transform_changed();
		});
	}

	const Transform& SceneComponentProxy::world_transform() const
	{
		return m_world_transform;
	}

	const Transform& SceneComponentProxy::local_transform() const
	{
		return m_local_transform;
	}

	SceneComponentProxy& SceneComponentProxy::world_transform(const Transform& transform)
	{
		m_world_transform = transform;
		return *this;
	}

	SceneComponentProxy& SceneComponentProxy::local_transform(const Transform& transform)
	{
		m_local_transform = transform;
		return *this;
	}

	SceneComponent::SceneComponent()
	{}

	SceneComponent& SceneComponent::attach(SceneComponent* child)
	{
		trinex_check(child != this, "Cannot attach a component to itself");
		trinex_check(child && !is_attached_to(child), "Setting up attachment would create a cycle");

		detach_from_parent();

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

	const Vector<Pointer<SceneComponent>>& SceneComponent::childs() const
	{
		return m_childs;
	}

	SceneComponent& SceneComponent::on_transform_changed()
	{
		m_is_dirty = true;

		submit_transform_to_render_thread();

		for (SceneComponent* child : m_childs)
		{
			child->on_transform_changed();
		}

		return *this;
	}

	SceneComponent* SceneComponent::parent() const
	{
		return m_parent.ptr();
	}

	SceneComponent& SceneComponent::destroyed()
	{
		detach_from_parent();
		Super::destroyed();

		return *this;
	}

	ActorComponentProxy* SceneComponent::create_proxy()
	{
		return new SceneComponentProxy();
	}

	void SceneComponent::submit_transform_to_render_thread()
	{
		if (SceneComponentProxy* component_proxy = proxy())
		{
			Thread* thread = render_thread();
			thread->create_task<UpdateVariableCommand<Transform>>(local_transform(), component_proxy->m_local_transform);
			thread->create_task<UpdateVariableCommand<Transform>>(world_transform(), component_proxy->m_world_transform);
		}
	}

	SceneComponent& SceneComponent::start_play()
	{
		Super::start_play();
		submit_transform_to_render_thread();
		return *this;
	}

	SceneComponentProxy* SceneComponent::proxy() const
	{
		return typed_proxy<SceneComponentProxy>();
	}

	const Transform& SceneComponent::local_transform() const
	{
		is_in_logic_thread_checked();
		return m_local;
	}

	const Transform& SceneComponent::world_transform() const
	{
		is_in_logic_thread_checked();

		if (m_is_dirty)
		{
			if (SceneComponent* parent_component = parent())
			{
				m_world = parent_component->world_transform() * m_local;
			}
			else
			{
				m_world = local_transform();
			}
			m_is_dirty = false;
		}

		return m_world;
	}

	SceneComponent& SceneComponent::local_transform(const Transform& transform)
	{
		is_in_logic_thread_checked();
		m_local = transform;
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_local_transform(const Transform& transform)
	{
		is_in_logic_thread_checked();
		m_local += transform;
		on_transform_changed();

		return *this;
	}

	SceneComponent& SceneComponent::remove_local_transform(const Transform& transform)
	{
		is_in_logic_thread_checked();
		m_local -= transform;
		on_transform_changed();

		return *this;
	}

	SceneComponent& SceneComponent::location(const Vector3D& new_location)
	{
		is_in_logic_thread_checked();
		m_local.location(new_location);
		on_transform_changed();

		return *this;
	}

	SceneComponent& SceneComponent::rotation(const Quaternion& new_rotation)
	{
		is_in_logic_thread_checked();
		m_local.rotation(new_rotation);


		return *this;
	}

	SceneComponent& SceneComponent::rotation(const Vector3D& new_rotation)
	{
		is_in_logic_thread_checked();
		m_local.rotation(new_rotation);
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::scale(const Vector3D& new_scale)
	{
		is_in_logic_thread_checked();
		m_local.scale(new_scale);
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_location(const Vector3D& delta)
	{
		is_in_logic_thread_checked();
		m_local.add_location(delta);
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_rotation(const Vector3D& delta)
	{
		is_in_logic_thread_checked();
		m_local.add_rotation(delta);
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_rotation(const Quaternion& delta)
	{
		is_in_logic_thread_checked();
		m_local.add_rotation(delta);
		on_transform_changed();
		return *this;
	}

	SceneComponent& SceneComponent::add_scale(const Vector3D& delta)
	{
		is_in_logic_thread_checked();
		m_local.add_scale(delta);
		on_transform_changed();
		return *this;
	}


}// namespace Engine
