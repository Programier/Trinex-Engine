#pragma once
#include <Core/export.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/hitbox.hpp>

namespace Engine
{
    class Scene;
    class SceneTreeNode;
    CLASS BasicDrawable : public virtual Object
    {
        declare_instance_info_hpp(BasicDrawable);

    public:
        delete_copy_constructors(BasicDrawable);
        constructor_hpp(BasicDrawable);
        virtual std::size_t render(const Matrix4f& matrix) = 0;
        virtual ~BasicDrawable();
    };

    CLASS Drawable : public BasicObject<Translate, Rotate, Scale>, public BasicDrawable
    {
    public:
        using SceneNodesMap = std::unordered_map<const Scene*, SceneTreeNode*>;
        enum AABB_UpdateMask : BitMask
        {
            None = 0,
            OnRotate = 1,
            OnScale = 2,
            All = 3
        };

    private:
        BoxHB _M_aabb[2];// {Original, Modified}
        BitMask _M_aabb_update_mask = Drawable::AABB_UpdateMask::All;
        SceneNodesMap _M_scene_nodes;
        void update_aabb(bool is_original_modified);

        declare_instance_info_hpp(Drawable);

    public:
        delete_copy_constructors(Drawable);
        constructor_hpp(Drawable);
        const BoxHB& aabb() const;
        Drawable& aabb(const BoxHB& box, bool is_original = true);
        const SceneNodesMap& scene_nodes() const;
        SceneTreeNode* find_scene_node(const Scene* scene);
        const SceneTreeNode* find_scene_node(const Scene* scene) const;

        friend class Scene;
        virtual ~Drawable();
    };
}// namespace Engine
