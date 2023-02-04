#include <Core/check.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/scene.hpp>
#include <list>

namespace Engine
{
    static Scene* _M_active_scene;
    SceneTreeNode::SceneTreeNode(Scene* scene)
    {
        check(scene);
        _M_scene = scene;
        name("Scene");
    }

    SceneTreeNode::SceneTreeNode(SceneTreeNode* parent) : Tree(parent)
    {
        check(parent);
        _M_scene = parent->_M_scene;
    }

    std::size_t SceneTreeNode::render(const Matrix4f& matrix)
    {
        return 0;
    }

    const Scene* SceneTreeNode::scene() const
    {
        return _M_scene;
    }

    Scene* SceneTreeNode::scene()
    {
        return _M_scene;
    }

    Matrix4f SceneTreeNode::global_matrix() const
    {
        Matrix4f matrix = Constants::identity_matrix;
        const SceneTreeNode* current = this;
        std::list<const SceneTreeNode*> stack;
        while (current)
        {
            stack.push_back(current);
            current = dynamic_cast<const SceneTreeNode*>(current->parent());
        }

        while (!stack.empty())
        {
            matrix *= stack.back()->model();
            stack.pop_back();
        }
        return matrix;
    }

    declare_instance_info_cpp(Scene);

    constructor_cpp(Scene)
    {}

    Scene::Scene(float octree_box_min_size) : _M_octree(octree_box_min_size)
    {}

    std::size_t Scene::render() const
    {
        if (!_M_active_camera)
            return 0;

        std::list<Scene::OctreeNode*> stack;

        if (_M_octree.head())
            stack.push_back(_M_octree.head());

        Frustum frustum(*_M_active_camera);
        std::size_t count = 0;

        while (!stack.empty())
        {
            Scene::OctreeNode* node = stack.back();
            stack.pop_back();

            if (node->box().is_in_frustum(frustum))
            {
                for (Drawable* object : node->values)
                {
                    const SceneTreeNode* scene_node = object->find_scene_node(this);
                    if (!scene_node)
                        continue;

                    auto global_matrix = scene_node->global_matrix();

                    //if (object->aabb().apply_model(global_matrix).is_in_frustum(frustum))
                    if (true)
                    {
                        count += object->render(global_matrix);
                    }
                    else
                    {
                        int a = 0;
                        a++;
                    }
                }


                for (byte i = 0; i < 8; i++)
                {
                    auto new_node = node->get(i);
                    if (new_node)
                        stack.push_back(new_node);
                }
            }
        }
        return count;
    }

    Scene& Scene::clear()
    {
        _M_drawable_set.clear();
        _M_octree.clear();
        _M_head->clear();
        return *this;
    }

    void Scene::remove_from_octree(Drawable* drawable)
    {
        SceneTreeNode* scene_node = drawable->find_scene_node(this);
        if (!scene_node)
            return;

        auto octree_node = _M_octree.find(drawable->aabb().apply_model(scene_node->global_matrix()));
        if (octree_node)
        {
            octree_node->values.erase(drawable);
            if (octree_node->values.empty())
                delete octree_node;
        }
    }

    void Scene::push_to_octree(Drawable* drawable)
    {
        SceneTreeNode* scene_node = drawable->find_scene_node(this);
        if (!scene_node)
            return;

        auto box = drawable->aabb().apply_model(scene_node->global_matrix());
        _M_octree.push(drawable, box);
    }

    Scene& Scene::push(Drawable* drawable, SceneTreeNode* node)
    {
        if (node->_M_scene != this)
            return *this;

        if (!node)
            node = _M_head;

        if (drawable && !contains(drawable))
        {
            node->value.insert(drawable);
            drawable->_M_scene_nodes[this] = node;
            push_to_octree(drawable);
            _M_drawable_set.insert(drawable);
        }
        return *this;
    }

    Scene& Scene::remove(Drawable* drawable)
    {
        if (contains(drawable))
        {
            _M_drawable_set.erase(drawable);
            remove_from_octree(drawable);
            auto node = drawable->find_scene_node(this);
            if (node)
            {
                node->value.erase(drawable);
                while (node->value.empty() && node != _M_head)
                {
                    auto parent = dynamic_cast<SceneTreeNode*>(node->parent());
                    delete node;
                    node = parent;
                }
            }
            drawable->_M_scene_nodes.erase(this);
        }

        return *this;
    }

    bool Scene::contains(Drawable* drawable)
    {
        return drawable ? _M_drawable_set.contains(drawable) : false;
    }

    Scene& Scene::load(const std::string& filename, LoadFunction loader)
    {
        loader(filename, this);
        return *this;
    }

    Scene::~Scene()
    {
        clear();
    }

    SceneTreeNode* Scene::scene_head()
    {
        return _M_head;
    }

    const SceneTreeNode* Scene::scene_head() const
    {
        return _M_head;
    }

    const Scene::DrawableSet& Scene::drawables() const
    {
        return _M_drawable_set;
    }

    ENGINE_EXPORT Scene* Scene::get_active_scene()
    {
        return _M_active_scene;
    }

    Scene& Scene::set_as_active_scene()
    {
        _M_active_scene = this;
        return *this;
    }

    const Scene::CamerasSet& Scene::cameras() const
    {
        return _M_cameras;
    }

    Scene& Scene::add_camera(Camera* camera)
    {
        if (camera)
        {
            _M_cameras.insert(camera);
            if (!_M_active_camera)
                _M_active_camera = camera;
        }

        return *this;
    }

    Scene& Scene::active_camera(Camera* camera)
    {
        _M_cameras.insert(camera);
        _M_active_camera = camera;
        return *this;
    }

    Camera* Scene::active_camera()
    {
        return _M_active_camera;
    }

    const Camera* Scene::active_camera() const
    {
        return _M_active_camera;
    }

    const Scene::SceneOctree& Scene::octree() const
    {
        return _M_octree;
    }
}// namespace Engine
