#include <Core/math/box.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>

namespace Engine
{
	Scene::Scene()
	{
		m_root_component = Object::new_instance<SceneComponent>("Root");
	}

	Scene& Scene::add_primitive(PrimitiveComponent* primitive)
	{
		m_primitive_octree.push(primitive->bounding_box(), primitive);
		return *this;
	}

	Scene& Scene::remove_primitive(PrimitiveComponent* primitive)
	{
		m_primitive_octree.remove(primitive->bounding_box(), primitive);
		return *this;
	}

	Scene& Scene::update_primitive_transform(PrimitiveComponent* primitive)
	{
		remove_primitive(primitive);
		primitive->update_bounding_box();
		add_primitive(primitive);
		return *this;
	}

	Scene& Scene::add_light(LightComponent* light)
	{
		if (light->light_type() == LightComponent::Directional)
		{
			m_directional_lights.insert(light);
		}
		else
		{
			Box3f box = light->bounding_box();
			m_light_octree.push(box, light);
		}

		return *this;
	}

	Scene& Scene::remove_light(LightComponent* light)
	{
		if (light->light_type() == LightComponent::Directional)
		{
			m_directional_lights.erase(light);
		}
		else
		{
			Box3f box = light->bounding_box();
			m_light_octree.remove(box, light);
		}
		return *this;
	}

	Scene& Scene::update_light_transform(LightComponent* light)
	{
		if (light->light_type() != LightComponent::Directional)
		{
			remove_light(light);
			light->update_bounding_box();
			add_light(light);
		}
		return *this;
	}

	Scene& Scene::add_post_process(PostProcessComponent* post_process)
	{
		m_unbound_post_processes.insert(post_process);
		return *this;
	}

	Scene& Scene::remove_post_process(PostProcessComponent* post_process)
	{
		m_unbound_post_processes.erase(post_process);
		return *this;
	}

	Scene& Scene::update_post_process_transform(PostProcessComponent* post_process)
	{
		return *this;
	}

	SceneComponent* Scene::root_component() const
	{
		return m_root_component.ptr();
	}

	template<typename Node, typename Container>
	static void collect_elements_internal(Node* node, Container& result)
	{
		for (auto component : node->values())
		{
			if (component->is_visible())
				result.emplace_back(component);
		}

		for (byte i = 0; i < 8; i++)
		{
			auto child = node->child_at(i);

			if (child)
			{
				collect_elements_internal(child, result);
			}
		}
	}

	template<typename Node, typename Container>
	static void collect_elements_internal(Node* node, const Frustum& frustum, Container& result)
	{
		if (node->size() == 0)
			return;

		auto type = frustum.containtment_type(node->box());

		if (type == Frustum::Outside)
			return;

		if (type == Frustum::Contains)
			return collect_elements_internal(node, result);

		for (auto component : node->values())
		{
			if (component->is_visible() && frustum.intersects(component->bounding_box()))
			{
				result.emplace_back(component);
			}
		}

		for (byte i = 0; i < 8; i++)
		{
			auto child = node->child_at(i);

			if (child)
			{
				collect_elements_internal(child, frustum, result);
			}
		}
	}

	FrameVector<PrimitiveComponent*> Scene::collect_visible_primitives(const Frustum& frustum)
	{
		FrameVector<PrimitiveComponent*> objects;
		objects.reserve(glm::max<size_t>(64, m_primitive_octree.size() / 10));
		collect_elements_internal(m_primitive_octree.root_node(), frustum, objects);
		return objects;
	}

	FrameVector<LightComponent*> Scene::collect_visible_lights(const Frustum& frustum)
	{
		FrameVector<LightComponent*> objects;
		objects.reserve(glm::max<size_t>(64, m_light_octree.size() / 10 + m_directional_lights.size()));
		collect_elements_internal(m_light_octree.root_node(), frustum, objects);
		for (LightComponent* light : m_directional_lights) objects.emplace_back(light);
		return objects;
	}

	FrameVector<PostProcessComponent*> Scene::collect_post_processes(const Vector3f& location)
	{
		FrameVector<PostProcessComponent*> objects;
		objects.reserve(m_unbound_post_processes.size());
		for (PostProcessComponent* component : m_unbound_post_processes) objects.push_back(component);
		return objects;
	}

	const Box3f& Scene::bounds() const
	{
		return m_primitive_octree.root_node()->box();
	}
}// namespace Engine
