#include <Core/math/box.hpp>
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
		Box3f m_box;

	public:
		AddPrimitiveTask(OctreeType* octree, typename OctreeType::ValueType primitive, const Box3f& box)
		    : m_octree(octree), m_primitive(primitive), m_box(box)
		{}

		void execute() override { m_octree->push(m_box, m_primitive); }
	};

	template<typename OctreeType>
	class RemovePrimitiveTask : public Task<RemovePrimitiveTask<OctreeType>>
	{
		OctreeType* m_octree;
		typename OctreeType::ValueType m_primitive;
		Box3f m_box;

	public:
		RemovePrimitiveTask(OctreeType* octree, typename OctreeType::ValueType primitive, const Box3f& box)
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
		render_thread()->create_task<AddPrimitiveTask<Scene::PrimitiveOctree>>(&m_primitive_octree, primitive,
		                                                                       primitive->bounding_box());
		return *this;
	}

	Scene& Scene::remove_primitive(PrimitiveComponent* primitive)
	{
		render_thread()->create_task<RemovePrimitiveTask<Scene::PrimitiveOctree>>(&m_primitive_octree, primitive,
		                                                                          primitive->bounding_box());
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
		render_thread()->create_task<AddPrimitiveTask<Scene::LightOctree>>(&m_light_octree, light, light->bounding_box());
		return *this;
	}

	Scene& Scene::remove_light(LightComponent* light)
	{
		render_thread()->create_task<RemovePrimitiveTask<Scene::LightOctree>>(&m_light_octree, light, light->bounding_box());
		return *this;
	}

	Scene& Scene::update_light_transform(LightComponent* light)
	{
		remove_light(light);
		light->update_bounding_box();
		add_light(light);
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
		objects.reserve(glm::max<size_t>(64, m_primitive_octree.size() / 10));
		collect_elements_internal(m_primitive_octree.root_node(), frustum, objects);
		return objects;
	}

	FrameVector<LightComponent*> Scene::collect_visible_lights(const Frustum& frustum)
	{
		FrameVector<LightComponent*> objects;
		objects.reserve(glm::max<size_t>(64, m_light_octree.size() / 10));
		collect_elements_internal(m_light_octree.root_node(), frustum, objects);
		return objects;
	}

	FrameVector<PostProcessComponent*> Scene::collect_post_processes(const Vector3f& location)
	{
		FrameVector<PostProcessComponent*> objects;
		objects.reserve(m_unbound_post_processes.size());
		for (PostProcessComponent* component : m_unbound_post_processes) objects.push_back(component);
		return objects;
	}

}// namespace Engine
