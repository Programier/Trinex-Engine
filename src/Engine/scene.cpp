#include <Core/math/box.hpp>
#include <Core/profiler.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>

namespace Trinex
{
	u32 Scene::add_primitive(PrimitiveComponent* primitive, const Box3i& box)
	{
		trinex_profile_cpu_n("Scene::add_primitive");
		return m_primitive_octree.insert(primitive, box);
	}

	Scene& Scene::update_primitive(u32 id, const Box3i& box)
	{
		trinex_profile_cpu_n("Scene::update_primitive");
		m_primitive_octree.update(id, box);
		return *this;
	}

	Scene& Scene::remove_primitive(u32 id)
	{
		trinex_profile_cpu_n("Scene::remove_primitive");
		m_primitive_octree.remove(id);
		return *this;
	}


	u32 Scene::add_light(LightComponent* light, const Box3i& box)
	{
		if (light->light_type() == LightComponent::Directional)
		{
			m_directional_lights.insert(light);
			return 0xFFFFFFFF;
		}
		else
		{
			return m_light_octree.insert(light, box);
		}
	}

	Scene& Scene::update_light(u32 id, const Box3i& box)
	{
		// if (light->light_type() != LightComponent::Directional)
		// {
		// 	remove_light(light);
		// 	light->update_bounding_box();
		// 	add_light(light);
		// }
		return *this;
	}

	Scene& Scene::remove_light(u32 id)
	{
		// if (light->light_type() == LightComponent::Directional)
		// {
		// 	m_directional_lights.erase(light);
		// }
		//else
		{
			m_light_octree.remove(id);
		}
		return *this;
	}

	u32 Scene::add_post_process(PostProcessComponent* post_process, const Box3i& box)
	{
		m_unbound_post_processes.insert(post_process);
		return 0;
	}

	Scene& Scene::update_post_process(u32 id, const Box3i& box)
	{
		return *this;
	}

	Scene& Scene::remove_post_process(u32 id)
	{
		return *this;
	}


	template<typename Component, typename Container>
	static void collect_elements_internal(Octree* octree, Octree::Node* node, Container& result)
	{
		for (auto id : node->value_ids())
		{
			Component* component = octree->value<Component>(id);

			if (component->is_visible())
				result.emplace_back(component);
		}

		for (u8 i = 0; i < 8; i++)
		{
			auto child = node->child(i);

			if (child)
			{
				collect_elements_internal<Component, Container>(octree, child, result);
			}
		}
	}

	template<typename Component, typename Container>
	static void collect_elements_internal(Octree* octree, Octree::Node* node, const Frustum& frustum, Container& result)
	{
		if (node->size() == 0)
			return;

		auto type = frustum.containtment_type(node->box());

		if (type == Frustum::Outside)
			return;

		if (type == Frustum::Contains)
			return collect_elements_internal<Component, Container>(octree, node, result);

		for (auto id : node->value_ids())
		{
			Component* component = octree->value<Component>(id);

			if (component->is_visible() && frustum.intersects(component->bounding_box()))
			{
				result.emplace_back(component);
			}
		}

		for (u8 i = 0; i < 8; i++)
		{
			auto child = node->child(i);

			if (child)
			{
				collect_elements_internal<Component, Container>(octree, child, frustum, result);
			}
		}
	}

	template<typename Component, typename Container>
	static FORCE_INLINE void collect_elements_internal(Octree* octree, const Frustum& frustum, Container& result)
	{
		collect_elements_internal<Component, Container>(octree, octree->root(), frustum, result);
	}

	FrameVector<PrimitiveComponent*> Scene::collect_visible_primitives(const Frustum& frustum)
	{
		FrameVector<PrimitiveComponent*> objects;
		objects.reserve(glm::max<usize>(64, m_primitive_octree.size() / 10));
		collect_elements_internal<PrimitiveComponent>(&m_primitive_octree, frustum, objects);
		return objects;
	}

	FrameVector<LightComponent*> Scene::collect_visible_lights(const Frustum& frustum)
	{
		FrameVector<LightComponent*> objects;
		objects.reserve(glm::max<usize>(64, m_light_octree.size() / 10 + m_directional_lights.size()));
		collect_elements_internal<LightComponent>(&m_light_octree, frustum, objects);
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

	const Box3i& Scene::bounds() const
	{
		return m_primitive_octree.root()->box();
	}
}// namespace Trinex
