#pragma once
#include <Core/pointer.hpp>
#include <Core/transform.hpp>
#include <Engine/ActorComponents/actor_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT SceneComponent : public ActorComponent
	{
		trinex_declare_class(SceneComponent, ActorComponent);

	private:
		Transform m_local;
		mutable Transform m_world;
		Transform m_prev_world;

		mutable bool m_is_transform_dirty : 1;
		bool m_is_transform_changed : 1;

		Pointer<SceneComponent> m_parent = nullptr;
		Vector<Pointer<SceneComponent>> m_childs;

	protected:
		void script_on_transform_changed();

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
		SceneComponent& destroyed() override;

		const Transform& world_transform() const;
		inline const Transform& local_transform() const { return m_local; }
		inline const Transform& previous_world_transform() const { return m_prev_world; }
		inline bool is_transform_changed() const { return m_is_transform_changed; }

		SceneComponent& local_transform(const Transform&);
		SceneComponent& add_local_transform(const Transform&);
		SceneComponent& remove_local_transform(const Transform&);
		SceneComponent& location(const Vector3f&);
		SceneComponent& rotation(const Quaternion&);
		SceneComponent& scale(const Vector3f&);
		SceneComponent& add_location(const Vector3f& delta);
		SceneComponent& add_rotation(const Quaternion& delta);
		SceneComponent& add_scale(const Vector3f& delta);
		SceneComponent& look_at(const Vector3f& location, const Vector3f& up = {0.f, 1.f, 0.f});

		inline SceneComponent* parent() const { return m_parent.ptr(); }
		inline const Vector<Pointer<SceneComponent>>& childs() const { return m_childs; }

		virtual SceneComponent& on_transform_changed();
		SceneComponent& on_property_changed(const Refl::PropertyChangedEvent& event) override;
	};
}// namespace Engine
