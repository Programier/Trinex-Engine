#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/scene.hpp>


namespace Engine
{
    SceneInterface::~SceneInterface()
    {}


    class AddPrimitiveTask : public ExecutableObject
    {
        Scene::SceneOctree* _M_octree;
        PrimitiveComponent* _M_primitive;
        AABB_3Df _M_box;

    public:
        AddPrimitiveTask(Scene::SceneOctree* octree, PrimitiveComponent* primitive, const AABB_3Df& box)
            : _M_octree(octree), _M_primitive(primitive), _M_box(box)
        {}

        int_t execute() override
        {
            _M_octree->push(_M_box, _M_primitive);
            return sizeof(AddPrimitiveTask);
        }
    };

    class RemovePrimitiveTask : public ExecutableObject
    {
        Scene::SceneOctree* _M_octree;
        PrimitiveComponent* _M_primitive;
        AABB_3Df _M_box;

    public:
        RemovePrimitiveTask(Scene::SceneOctree* octree, PrimitiveComponent* primitive, const AABB_3Df& box)
            : _M_octree(octree), _M_primitive(primitive), _M_box(box)
        {}

        int_t execute() override
        {
            _M_octree->remove(_M_box, _M_primitive);
            return sizeof(AddPrimitiveTask);
        }
    };


    Scene::Scene()
    {
        _M_root_component = Object::new_instance_named<SceneComponent>("Root");
    }

    Scene& Scene::add_primitive(PrimitiveComponent* primitive)
    {
        render_thread()->insert_new_task<AddPrimitiveTask>(&_M_octree, primitive, primitive->bounding_box());
        return *this;
    }

    Scene& Scene::remove_primitive(PrimitiveComponent* primitive)
    {
        render_thread()->insert_new_task<RemovePrimitiveTask>(&_M_octree, primitive, primitive->bounding_box());
        return *this;
    }


    SceneComponent* Scene::root_component() const
    {
        return _M_root_component.ptr();
    }
}// namespace Engine
