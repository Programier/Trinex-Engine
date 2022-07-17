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

            Node* push(const Type& value, const AABB_3D& box, Node* head = nullptr);
            Node* push_to_head(const Type& value, const AABB_3D& box, int mode);
            ~Node();
        };

        Node* _M_head = nullptr;

    public:
        Octree();
        Octree& push(const Type& value, const AABB_3D& box);
        AABB_3D aabb() const;
        ~Octree();
    };


    //      Node implementation

    template<typename Type>
    typename Octree<Type>::Node* Octree<Type>::Node::push_to_head(const Type& value, const AABB_3D& box, int mode)
    {
        Node* head = new Node;

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
        head->_M_box.min = limits[0];
        head->_M_box.max = (limits[1] * 2.f) - limits[0];

        for (int i = 0; i < 3; i++)
            if (head->_M_box.min[i] > head->_M_box.max[i])
                std::swap(head->_M_box.min[i], head->_M_box.max[i]);

        this->_M_prev = head;

        head->_M_objects.push_back(value);
        head->_M_parts[mode >> 3] = this;
        DEBUG_CODE(std::clog << "Octree: Pushing leaf at block with num " << (mode >> 3) << std::endl;);
        return head;
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

        for (int i = 0; i < 3; i++) (push_to <<= 1) += cast(int, _M_box.min[i] > box.min[i]);
        for (int i = 0; i < 3; i++) (push_to <<= 1) += cast(int, _M_box.max[i] < box.max[i]);


        if (push_to)
            return this->push_to_head(value, box, push_to);

        while (this_object)
        {
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
                    new_aabb.max[i] = _M_box.max[i];
                }
                else
                {
                    new_aabb.max[i] = center[i];
                    new_aabb.min[i] = _M_box.min[i];
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
    Octree<Type>::~Octree()
    {
        if (_M_head)
            delete _M_head;
    }

}// namespace Engine
