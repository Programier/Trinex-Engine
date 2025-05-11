#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>


namespace Engine
{
	template<typename OctreeType>
	class AddPrimitiveTask : public Task<AddPrimitiveTask<OctreeType>>
	{
		OctreeType* m_octree;
		typename OctreeType::ValueType m_primitive;
		AABB_3Df m_box;

	public:
		AddPrimitiveTask(OctreeType* octree, typename OctreeType::ValueType primitive, const AABB_3Df& box)
		    : m_octree(octree), m_primitive(primitive), m_box(box)
		{}

		void execute() override { m_octree->push(m_box, m_primitive); }
	};

	template<typename OctreeType>
	class RemovePrimitiveTask : public Task<RemovePrimitiveTask<OctreeType>>
	{
		OctreeType* m_octree;
		typename OctreeType::ValueType m_primitive;
		AABB_3Df m_box;

	public:
		RemovePrimitiveTask(OctreeType* octree, typename OctreeType::ValueType primitive, const AABB_3Df& box)
		    : m_octree(octree), m_primitive(primitive), m_box(box)
		{}

		void execute() override { m_octree->remove(m_box, m_primitive); }
	};

	Scene::Scene()
	{
		m_root_component = Object::new_instance<SceneComponent>("Root");
	}

	Scene& Scene::add_primitive(PrimitiveComponent* primitive)
	{
		render_thread()->create_task<AddPrimitiveTask<Scene::PrimitiveOctree>>(&m_octree_render_thread, primitive,
		                                                                       primitive->bounding_box());
		m_octree.push(primitive->bounding_box(), primitive);
		return *this;
	}

	Scene& Scene::remove_primitive(PrimitiveComponent* primitive)
	{
		render_thread()->create_task<RemovePrimitiveTask<Scene::PrimitiveOctree>>(&m_octree_render_thread, primitive,
		                                                                          primitive->bounding_box());
		m_octree.remove(primitive->bounding_box(), primitive);
		return *this;
	}

	Scene& Scene::update_primitive_transform(PrimitiveComponent* primitive)
	{
		remove_primitive(primitive);
		primitive->update_bounding_box();
		add_primitive(primitive);
		return *this;
	}

	Scene& Scene::update_light_transform(LightComponent* light)
	{
		remove_light(light);
		light->update_bounding_box();
		add_light(light);
		return *this;
	}

	Scene& Scene::add_light(LightComponent* light)
	{
		render_thread()->create_task<AddPrimitiveTask<Scene::LightOctree>>(&m_light_octree_render_thread, light,
		                                                                   light->bounding_box());
		m_light_octree.push(light->bounding_box(), light);
		return *this;
	}

	Scene& Scene::remove_light(LightComponent* light)
	{
		render_thread()->create_task<RemovePrimitiveTask<Scene::LightOctree>>(&m_light_octree_render_thread, light,
		                                                                      light->bounding_box());
		m_light_octree.remove(light->bounding_box(), light);
		return *this;
	}

	SceneComponent* Scene::root_component() const
	{
		return m_root_component.ptr();
	}

	const Scene::PrimitiveOctree& Scene::primitive_octree() const
	{
		if (is_in_render_thread())
		{
			return m_octree_render_thread;
		}
		else
		{
			return m_octree;
		}
	}

	const Scene::LightOctree& Scene::light_octree() const
	{
		if (is_in_render_thread())
		{
			return m_light_octree_render_thread;
		}
		else
		{
			return m_light_octree;
		}
	}

	template<typename Node, typename Container>
	static void collect_elements_internal(Node* node, const Frustum& frustum, Container& result)
	{
		if (node->size() == 0 || !frustum.in_frustum(node->box()))
			return;

		for (auto component : node->values())
		{
			if (frustum.in_frustum(component->bounding_box()))
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
		const PrimitiveOctree& octree = primitive_octree();
		objects.reserve(glm::max<size_t>(64, octree.size() / 10));

		collect_elements_internal(octree.root_node(), frustum, objects);
		return objects;
	}

	FrameVector<LightComponent*> Scene::collect_visible_lights(const Frustum& frustum)
	{
		FrameVector<LightComponent*> objects;
		const LightOctree& octree = light_octree();
		objects.reserve(glm::max<size_t>(64, octree.size() / 10));

		collect_elements_internal(octree.root_node(), frustum, objects);
		return objects;
	}

	Scene::~Scene() {}
}// namespace Engine
