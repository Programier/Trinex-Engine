#pragma once
#include <unordered_set>
#include <list>

namespace Engine
{
    template<typename Type>
    class Tree
    {
    public:
        using Node = Tree;
        using NodeList = std::unordered_set<Node*>;

    private:
        NodeList _M_nodes;
        Node* _M_parent = nullptr;


    public:
        Type value;

        Tree() = default;
        Tree(Tree* tree) : _M_parent(tree)
        {
            if (tree)
                tree->_M_nodes.insert(this);
        }

        Tree* copy() const
        {
            Tree* tree = new Tree();
            tree->value = value;
            for (Node* node : _M_nodes) tree->_M_nodes.insert(node->copy());

            for (Node* node : tree->_M_nodes) node->_M_parent = tree;
        }

        const NodeList& nodes() const
        {
            return _M_nodes;
        }

        Node* parent()
        {
            return _M_parent;
        }

        const Node* parent() const
        {
            return _M_parent;
        }

        Tree& clear()
        {
            //for (auto node : std::list(_M_nodes.begin(), _M_nodes.end()));
            return *this;
        }

        virtual ~Tree()
        {
            if (_M_parent)
                _M_parent->_M_nodes.erase(this);

            // Recursive delete subnodes
            for (auto node : std::list(_M_nodes.begin(), _M_nodes.end()))
            {
                delete node;
            }
        }
    };
}// namespace Engine
