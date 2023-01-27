#pragma once
#include <Core/callback.hpp>
#include <Core/export.hpp>
#include <Core/object.hpp>
#include <unordered_set>

namespace Engine
{
    class SceneNode : public Object
    {
    public:
        using NodesSet = std::unordered_set<SceneNode*>;

    protected:
        bool _M_is_active = true;
        bool _M_visible = true;
        PriorityIndex _M_process_priority = 0;
        SceneNode* _M_parent_node = nullptr;
        NodesSet _M_childs;
        virtual SceneNode& process(float dt) const;
        CallBacks<void, SceneNode*> _M_add_node_callback;
        CallBacks<void, SceneNode*> _M_remove_node_callback;

        declare_instance_info_hpp(SceneNode);

    public:
        delete_copy_constructors(SceneNode);
        constructor_hpp(SceneNode);

        bool active() const;
        bool visible() const;
        SceneNode& active(bool flag) const;
        SceneNode& visible(bool flag) const;

        PriorityIndex priority_index() const;
        SceneNode& priority_index(PriorityIndex index);
        SceneNode* parent_node() const;
        SceneNode& add_child_node(SceneNode* node);
        SceneNode& remove_child_node(SceneNode* node);
    };
}// namespace Engine
