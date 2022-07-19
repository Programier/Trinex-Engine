#pragma once

#include <BasicFunctional/debug.hpp>
#include <BasicFunctional/engine_types.hpp>
#include <engine.hpp>
#include <iostream>
#include <list>

namespace Engine
{
    template<typename Type>
    class Octree
    {
    private:
        struct Node {
            Node* _M_parts[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
            Node* _M_prev = nullptr;
            AABB_3D _M_box;

            std::list<Type> _M_objects;

        private:
            Node* push(const Type& value, const AABB_3D& box, Node* head = nullptr);
            AABB_3D calc_new_head(const Type& value, const AABB_3D& box, int mode);

        public:
            ~Node();
        };

        Node* _M_head = nullptr;


    public:
        Octree();
        Octree(const AABB_3D& box);
        Octree(const Octree& tree);
        Octree(Octree&& tree);
        Octree& operator=(const Octree& tree);
        Octree& push(const Type& value, const AABB_3D& box);
        Octree& clear();
        AABB_3D aabb() const;
        ~Octree();

    private:
        void copy_tree(Node*& to, Node* node, Node* prev = nullptr);
    };


    //      Node implementation

    template<typename Type>
    AABB_3D Octree<Type>::Node::calc_new_head(const Type& value, const AABB_3D& box, int mode)
    {
        AABB_3D aabb;
        // {min : center}
        Point3D limits[2] = {_M_box.min, _M_box.max};

        for (int i = 0; i < 3; i++)
        {
            bool bit = get_bit(mode, 6 - i);
            if (!bit)
                continue;
            std::swap(limits[1][i], limits[0][i]);
        }

        // Calculate AABB
        aabb.min = limits[0];
        aabb.max = (limits[1] * 2.f) - limits[0];

        for (int i = 0; i < 3; i++)
            if (aabb.min[i] > aabb.max[i])
                std::swap(aabb.min[i], aabb.max[i]);
        return aabb;
    }

    // Return head of octree
    template<typename Type>
    typename Octree<Type>::Node* Octree<Type>::Node::push(const Type& value, const AABB_3D& box, Node* head)
    {
        Node* this_object = this;
        if (!head)
            head = this_object;

        int push_to = 0;
        int min_value, max_value;
        int pow_value = 1;
        AABB_3D new_aabb;
        bool check_size = true;

        while (this_object)
        {
            push_to = 0;
            pow_value = 1;
            if (check_size)
            {
                for (int i = 0; i < 3; i++) (push_to <<= 1) += cast(int, this_object->_M_box.min[i] > box.min[i]);
                for (int i = 0; i < 3; i++) (push_to <<= 1) += cast(int, this_object->_M_box.max[i] < box.max[i]);

                if (push_to)
                {
                    Node* new_head = new Node;
                    new_head->_M_box = this_object->calc_new_head(value, box, push_to);
                    head->_M_prev = new_head;
                    new_head->_M_parts[push_to >> 3] = head;
                    head = new_head;
                    this_object = new_head;
                    continue;
                }
                else
                    check_size = false;
            }

            Point3D center = (this_object->_M_box.min + this_object->_M_box.max) / 2.f;
            push_to = -7;
            for (int i = 0; i < 3; i++)
            {
                min_value = cast(int, center[i] >= box.min[i]);
                max_value = cast(int, center[i] < box.max[i]);

                if (min_value == max_value)
                {
                    this_object->_M_objects.push_back(value);
                    return head;
                }

                if (max_value)
                {
                    new_aabb.min[i] = center[i];
                    new_aabb.max[i] = this_object->_M_box.max[i];
                }
                else
                {
                    new_aabb.max[i] = center[i];
                    new_aabb.min[i] = this_object->_M_box.min[i];
                }

                push_to += pow_value + max_value * pow_value;
                pow_value <<= 1;
            }

            Node* tmp = this_object->_M_parts[push_to];
            if (!tmp)
            {
                tmp = new Node;
                tmp->_M_prev = this_object;
                tmp->_M_box = new_aabb;
                this_object->_M_parts[push_to] = tmp;
            }

            this_object = tmp;
        }
        return head;
    }

    template<typename Type>
    Octree<Type>::Node::~Node()
    {
        // Recursive delete leaf
        for (int i = 0; i < 8; i++)
            if (_M_parts[i])
                delete _M_parts[i];
    }

    // Octree implementation

    template<typename Type>
    Octree<Type>::Octree() = default;

    template<typename Type>
    Octree<Type>::Octree(const AABB_3D& box)
    {
        _M_head = new Node;
        _M_head->_M_box = box;
    }

    template<typename Type>
    Octree<Type>::Octree(const Octree& tree)
    {
        *this = tree;
    }

    template<typename Type>
    Octree<Type>::Octree(Octree&& tree)
    {
        _M_head = std::move(tree._M_head);
        tree._M_head = nullptr;
    }

    template<typename Type>
    Octree<Type>& Octree<Type>::push(const Type& value, const AABB_3D& box)
    {
        if (!_M_head)
        {
            _M_head = new Node;
            _M_head->_M_box = box;
            _M_head->_M_objects.push_back(value);
            return *this;
        }

        _M_head = _M_head->push(value, box);
        return *this;
    }

    template<typename Type>
    AABB_3D Octree<Type>::aabb() const
    {
        return _M_head ? _M_head->_M_box : AABB_3D();
    }


    template<typename Type>
    Octree<Type>& Octree<Type>::operator=(const Octree<Type>& tree)
    {
        if (this == &tree)
            return *this;
        clear();
        copy_tree(_M_head, tree._M_head, nullptr);
    }

    // Recursive copy
    template<typename Type>
    void Octree<Type>::copy_tree(Node*& to, Node* node, Node* prev)
    {
        to = new Node;
        to->_M_box = node->_M_box;
        to->_M_prev = prev;
        to->_M_objects = node->_M_objects;
        for (int i = 0; i < 8; i++) copy_tree(to->_M_parts[i], node->_M_parts[i], to);
    }

    template<typename Type>
    Octree<Type>& Octree<Type>::clear()
    {
        if (_M_head)
            delete _M_head;
        _M_head = nullptr;
        return *this;
    }


    template<typename Type>
    Octree<Type>::~Octree()
    {
        if (_M_head)
            delete _M_head;
    }

}// namespace Engine
