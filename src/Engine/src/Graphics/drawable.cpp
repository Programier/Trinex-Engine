#include <Graphics/drawable.hpp>
#include <Graphics/resources.hpp>
#include <Graphics/scene.hpp>


namespace Engine
{
#define MODIFIED_AABB 0
#define ORIGINAL_AABB 1

    declare_instance_info_cpp(BasicDrawable);
    constructor_cpp(BasicDrawable)
    {}

    declare_instance_info_cpp(Drawable);
    constructor_cpp(Drawable)
    {
        Resources::drawables.push_back(this);
    }

    BasicDrawable::~BasicDrawable() = default;

    void Drawable::update_aabb(bool is_original_modified)
    {
        throw not_implemented;
        //        if (is_original_modified)
        //            _M_aabb[MODIFIED_AABB] = _M_aabb[ORIGINAL_AABB];
    }

    Drawable::~Drawable() = default;

    const BoxHB& Drawable::aabb() const
    {
        return _M_aabb[MODIFIED_AABB];
    }

    Drawable& Drawable::aabb(const BoxHB& box, bool is_original)
    {
        throw not_implemented;
        //update_aabb(is_original);
        return *this;
    }

    const Drawable::SceneNodesMap& Drawable::scene_nodes() const
    {
        return _M_scene_nodes;
    }

    SceneTreeNode* Drawable::find_scene_node(const Scene* scene)
    {
        auto iter = _M_scene_nodes.find(scene);
        if (iter == _M_scene_nodes.end())
            return nullptr;
        return (*iter).second;
    }

    const SceneTreeNode* Drawable::find_scene_node(const Scene* scene) const
    {
        auto iter = _M_scene_nodes.find(scene);
        if (iter == _M_scene_nodes.end())
            return nullptr;
        return (*iter).second;
    }
}// namespace Engine
