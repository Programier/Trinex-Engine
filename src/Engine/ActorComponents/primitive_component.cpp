#include <Core/base_engine.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	trinex_implement_engine_class(PrimitiveComponent, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_class_instance());

		r.method("void start_play()", trinex_scoped_void_method(This, start_play));
		r.method("void stop_play()", trinex_scoped_void_method(This, stop_play));
	}

	static const AABB_3Df default_bounds({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});

	PrimitiveComponent::PrimitiveComponent() : m_bounding_box(default_bounds) {}

	bool PrimitiveComponent::is_visible() const
	{
		if (m_is_visible)
		{
			if (auto owner_actor = actor())
				return owner_actor->is_visible();
			return true;
		}
		return false;
	}

	const AABB_3Df& PrimitiveComponent::bounding_box() const
	{
		return m_bounding_box;
	}

	PrimitiveComponent& PrimitiveComponent::is_visible(bool visible)
	{
		m_is_visible = visible;
		return *this;
	}

	PrimitiveComponent& PrimitiveComponent::start_play()
	{
		Super::start_play();
		if (Actor* owner_actor = actor())
		{
			if (World* world = owner_actor->world())
			{
				if (Scene* scene = world->scene())
				{
					scene->add_primitive(this);
				}
			}
		}
		return *this;
	}

	PrimitiveComponent& PrimitiveComponent::stop_play()
	{
		Super::stop_play();

		if (Actor* owner_actor = actor())
		{
			if (World* world = owner_actor->world())
			{
				if (Scene* scene = world->scene())
				{
					scene->remove_primitive(this);
				}
			}
		}

		return *this;
	}

	PrimitiveComponent& PrimitiveComponent::on_transform_changed()
	{
		Super::on_transform_changed();

		if (Scene* world_scene = scene())
		{
			world_scene->update_primitive_transform(this);
		}

		return update_bounding_box();
	}


	PrimitiveComponent& PrimitiveComponent::render(class SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
	}

	void PrimitiveComponent::submit_bounds_to_render_thread()
	{
		if (Proxy* component_proxy = proxy())
		{
			render_thread()->create_task<UpdateVariableCommand<AABB_3Df>>(m_bounding_box, component_proxy->m_bounds);
		}
	}

	PrimitiveComponent& PrimitiveComponent::update_bounding_box()
	{
		m_bounding_box = default_bounds.apply_transform(world_transform().matrix());
		submit_bounds_to_render_thread();
		return *this;
	}


	PrimitiveComponent::~PrimitiveComponent() {}
}// namespace Engine
