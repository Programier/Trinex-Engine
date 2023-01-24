#pragma once
#include <Core/export.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/hitbox.hpp>

namespace Engine
{
    class Scene;
    class SceneTreeNode;

    ENGINE_EXPORT class SceneObject : public BasicObject<Translate, Rotate, Scale>
    {
    public:
        using SceneNodesMap = std::unordered_map<const Scene*, SceneTreeNode*>;

    private:
        BoxHB _M_aabb;
        SceneNodesMap _M_scene_nodes;

    public:
        virtual const BoxHB& aabb() const = 0;
        SceneObject& aabb(const BoxHB& box, bool is_original = true);
        const SceneNodesMap& scene_nodes() const;
        SceneTreeNode* find_scene_node(const Scene* scene);
        const SceneTreeNode* find_scene_node(const Scene* scene) const;

        friend class Scene;
    };
}// namespace Engine


