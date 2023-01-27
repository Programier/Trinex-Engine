#pragma once

#include <Core/export.hpp>
#include <Core/object.hpp>
#include <Graphics/drawable.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/octree.hpp>
#include <TemplateFunctional/tree.hpp>


namespace Engine
{

    class Scene;
    CLASS SceneTreeNode : public Tree<std::unordered_set<Drawable*>>,
                          public BasicObject<Translate, Rotate, Scale>,
                          public BasicDrawable
    {
    private:
        Scene* _M_scene;

        SceneTreeNode(Scene * scene);

    public:
        const Scene* scene() const;
        Scene* scene();
        SceneTreeNode(SceneTreeNode * parent);
        std::size_t render(const Matrix4f& matrix) override;
        Matrix4f global_matrix() const;
        friend class Scene;
        friend class Object;
    };

    CLASS Scene final : public virtual Object
    {

    public:
        using OctreeNode = Octree<Drawable*>::OctreeNode;
        using LoadFunction = void (*)(const std::string& filename, Scene* scene);
        using DrawableSet = std::unordered_set<Drawable*>;
        using CamerasSet = std::unordered_set<Camera*>;
        using SceneOctree = Octree<Drawable*>;


    private:
        SceneOctree _M_octree;
        DrawableSet _M_drawable_set;

        void remove_from_octree(Drawable * drawable);
        void push_to_octree(Drawable * drawable);
        SceneTreeNode* _M_head = Object::new_instance<SceneTreeNode>(this);

        // Cameras block
        CamerasSet _M_cameras;
        Camera* _M_active_camera = nullptr;

        declare_instance_info_hpp(Scene);

    public:
        delete_copy_constructors(Scene);
        constructor_hpp(Scene);
        Scene(float octree_box_mi_size);

        std::size_t render() const;
        Scene& clear();
        Scene& push(Drawable * drawable, SceneTreeNode* node = nullptr);
        Scene& remove(Drawable * drawable);
        bool contains(Drawable * drawable);
        SceneTreeNode* scene_head();
        const SceneTreeNode* scene_head() const;
        const DrawableSet& drawables() const;

        Scene& load(const std::string& filename, LoadFunction loader);
        ENGINE_EXPORT static Scene* get_active_scene();
        Scene& set_as_active_scene();
        Scene& add_camera(Camera * camera);
        Scene& active_camera(Camera * camera);
        Camera* active_camera();
        const Camera* active_camera() const;
        const CamerasSet& cameras() const;
        const SceneOctree& octree() const;

        ~Scene();
    };
}// namespace Engine
