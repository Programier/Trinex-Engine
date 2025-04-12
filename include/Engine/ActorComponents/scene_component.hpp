#pragma once
#include <Core/pointer.hpp>
#include <Core/transform.hpp>
#include <Engine/ActorComponents/actor_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT SceneComponentProxy : public ActorComponentProxy
	{
		Transform m_world_transform;
		Transform m_local_transform;

	public:
		const Transform& world_transform() const;
		const Transform& local_transform() const;
		SceneComponentProxy& world_transform(const Transform& transform);
		SceneComponentProxy& local_transform(const Transform& transform);
		friend class SceneComponent;
	};

	class ENGINE_EXPORT SceneComponent : public ActorComponent
	{
		trinex_declare_class(SceneComponent, ActorComponent);

	private:
		mutable Transform m_local;
		mutable Transform m_world;
		mutable bool m_is_dirty;


		Pointer<SceneComponent> m_parent = nullptr;
		Vector<Pointer<SceneComponent>> m_childs;

	protected:
		void script_on_transform_changed();
		void submit_transform_to_render_thread();

	public:
		template<typename Native>
		struct Scriptable : Super::Scriptable<Native> {
			Scriptable& on_transform_changed() override
			{
				SceneComponent::script_on_transform_changed();
				return *this;
			}
		};

		SceneComponent();

		SceneComponent& attach(SceneComponent* child);
		SceneComponent& detach_from_parent();
		bool is_attached_to(SceneComponent* component) const;
		SceneComponent* parent() const;
		const Vector<Pointer<SceneComponent>>& childs() const;
		SceneComponentProxy* proxy() const;
		SceneComponent& destroyed() override;
		ActorComponentProxy* create_proxy() override;
		SceneComponent& start_play() override;


		const Transform& local_transform() const;
		const Transform& world_transform() const;

		SceneComponent& local_transform(const Transform&);
		SceneComponent& add_local_transform(const Transform&);
		SceneComponent& remove_local_transform(const Transform&);
		SceneComponent& location(const Vector3f&);
		SceneComponent& rotation(const Quaternion&);
		SceneComponent& rotation(const Vector3f&);
		SceneComponent& scale(const Vector3f&);
		SceneComponent& add_location(const Vector3f& delta);
		SceneComponent& add_rotation(const Vector3f& delta);
		SceneComponent& add_rotation(const Quaternion& delta);
		SceneComponent& add_scale(const Vector3f& delta);
		SceneComponent& look_at(const Vector3f& location, const Vector3f& up = {0.f, 1.f, 0.f});

		virtual SceneComponent& on_transform_changed();
	};
}// namespace Engine
