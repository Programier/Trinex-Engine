#pragma once

#include <BasicFunctional/debug.hpp>
#include <BasicFunctional/engine_types.hpp>
#include <engine.hpp>
#include <iostream>
#include <list>

#define TEMPLATE template<typename Type>

namespace Engine
{


    TEMPLATE
    class Octree
    {
    public:
        struct Index {
            bool x = false;
            bool y = false;
            bool z = false;

            int get() const;
            operator int() const;
        };

    private:
        struct Node {

        private:
            Octree* _M_tree = nullptr;
            AABB_3D calc_new_head(const AABB_3D& box, int mode);
            void set_base_class(Octree *tree);

        public:
            Node* _M_parts[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
            Node* _M_prev = nullptr;
            AABB_3D _M_box;
            std::list<Type> _M_objects;


            Node* push(const Type& value, const AABB_3D& box);
            Node* push_box(const AABB_3D& box);
            std::size_t depth() const;

            Node* get(const Octree::Index& index);
            Node& operator[](const Octree::Index& index);
            const Node* get(const Octree::Index& index) const;
            const Node& operator[](const Octree::Index& index) const;

            Octree& tree();
            const Octree& tree() const;

            friend class Octree;
            ~Node();
        };

        Node* _M_head = nullptr;
        std::size_t _M_size = 0;

        DEBUG_CODE(std::size_t _M_alloc_count = 0; std::size_t _M_dealloc_count = 0);


    public:
        Octree();
        Octree(const AABB_3D& box);
        Octree(const Octree& tree);
        Octree(Octree&& tree);
        Octree& operator=(const Octree& tree);
        Octree& operator=(Octree&& tree);
        Octree& push(const Type& value, const AABB_3D& box);
        Octree& clear();
        AABB_3D aabb() const;
        std::size_t depth() const;
        std::size_t size() const;

        Node* get(const Octree::Index& index);
        Node& operator[](const Octree::Index& index);
        const Node* get(const Octree::Index& index) const;
        const Node& operator[](const Octree::Index& index) const;


        DEBUG_CODE(std::size_t alloc_count() const; std::size_t dealloc_count() const);
        ~Octree();

    private:
        void copy_tree(Node*& to, Node* node, Node* prev = nullptr);
        void set_base_class();
        Node* new_node(Node* prev = nullptr, const AABB_3D& box = AABB_3D());
    };


    //  Index implementation
    TEMPLATE
    int Octree<Type>::Index::get() const
    {
        return cast(int, x) + 2 * cast(int, y) + 4 * cast(int, z);
    }

    TEMPLATE
    Octree<Type>::Index::operator int() const
    {
        return get();
    }

    //      Node implementation

    TEMPLATE
    std::size_t Octree<Type>::Node::depth() const
    {
        std::size_t d = 0;
        for (auto& node : _M_parts)
            if (node)
                d = std::max(d, node->depth());
        return 1 + d;
    }

    TEMPLATE
    AABB_3D Octree<Type>::Node::calc_new_head(const AABB_3D& box, int mode)
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

    TEMPLATE
    typename Octree<Type>::Node* Octree<Type>::Node::push_box(const AABB_3D& box)
    {
        Node* this_object = this;
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
                    if (!this_object->_M_prev)
                    {
                        Node* new_head = _M_tree->new_node(nullptr, this_object->calc_new_head(box, push_to));
                        this_object->_M_prev = new_head;
                        new_head->_M_parts[push_to >> 3] = this_object;
                        _M_tree->_M_head = new_head;
                    }

                    this_object = this_object->_M_prev;
                    continue;
                }
                else
                    check_size = false;
            }

            Point3D center = (this_object->_M_box.min + this_object->_M_box.max) / 2.f;
            push_to = 0;
            for (int i = 0; i < 3; i++)
            {
                min_value = cast(int, center[i] >= box.min[i]);
                max_value = cast(int, center[i] < box.max[i]);

                if (min_value == max_value)
                    return this_object;

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

                push_to += max_value * pow_value;
                pow_value <<= 1;
            }

            Node* tmp = this_object->_M_parts[push_to];
            if (!tmp)
                this_object->_M_parts[push_to] = tmp = _M_tree->new_node(this_object, new_aabb);

            this_object = tmp;
        }
        return this_object;
    }

    // Return node with value
    TEMPLATE
    typename Octree<Type>::Node* Octree<Type>::Node::push(const Type& value, const AABB_3D& box)
    {
        Node* node = push_box(box);
        node->_M_objects.push_back(value);
        _M_tree->_M_size++;
        return node;
    }

    TEMPLATE
    Octree<Type>::Node::~Node()
    {
        // Recursive delete leaf
        DEBUG_CODE(_M_tree->_M_dealloc_count++);
        for (int i = 0; i < 8; i++)
            if (_M_parts[i])
                delete _M_parts[i];
    }

    TEMPLATE
    typename Octree<Type>::Node* Octree<Type>::Node::get(const Octree<Type>::Index& index)
    {
        return _M_parts[index.get()];
    }

    TEMPLATE
    typename Octree<Type>::Node& Octree<Type>::Node::operator[](const Octree<Type>::Index& index)
    {
        Node* node = get(index);
        if (!node)
            throw std::runtime_error("Octree: Index out of range");
        return *node;
    }

    TEMPLATE
    const typename Octree<Type>::Node* Octree<Type>::Node::get(const Octree<Type>::Index& index) const
    {
        return _M_parts[index.get()];
    }

    TEMPLATE
    const typename Octree<Type>::Node& Octree<Type>::Node::operator[](const Octree<Type>::Index& index) const
    {
        Node* node = get(index);
        if (!node)
            throw std::runtime_error("Octree: Index out of range");
        return *node;
    }

    TEMPLATE
    Octree<Type>& Octree<Type>::Node::tree()
    {
        return *_M_tree;
    }

    TEMPLATE
    const Octree<Type>& Octree<Type>::Node::tree() const
    {
        return *_M_tree;
    }

    TEMPLATE
    void Octree<Type>::Node::set_base_class(Octree<Type>* tree)
    {
        _M_tree = tree;
        for(Node*& ell : _M_parts)
            if(ell)
                ell->set_base_class(tree);
    }



    // OCTREE IMPLEMENTATION

    TEMPLATE
    Octree<Type>::Octree() = default;

    TEMPLATE
    Octree<Type>::Octree(const AABB_3D& box)
    {
        _M_head = new_node(nullptr, box);
    }

    TEMPLATE
    Octree<Type>::Octree(const Octree& tree)
    {
        *this = tree;
    }

    TEMPLATE
    Octree<Type>::Octree(Octree&& tree)
    {
        *this = tree;
    }

    TEMPLATE
    Octree<Type>& Octree<Type>::push(const Type& value, const AABB_3D& box)
    {
        if (!_M_head)
        {
            _M_head = new_node(nullptr, box);
            _M_head->_M_objects.push_back(value);
            _M_size++;
            return *this;
        }

        _M_head->push(value, box);
        return *this;
    }

    TEMPLATE
    AABB_3D Octree<Type>::aabb() const
    {
        return _M_head ? _M_head->_M_box : AABB_3D();
    }


    TEMPLATE
    Octree<Type>& Octree<Type>::operator=(const Octree<Type>& tree)
    {
        if (this == &tree)
            return *this;
        clear();
        copy_tree(_M_head, tree._M_head, nullptr);
    }

    TEMPLATE
    Octree<Type>& Octree<Type>::operator=(Octree&& tree)
    {
        if (this == &tree)
            return *this;

        _M_head = std::move(tree._M_head);
        _M_size = std::move(tree._M_size);
        DEBUG_CODE(_M_alloc_count = std::move(tree._M_alloc_count); _M_dealloc_count = std::move(tree._M_dealloc_count));
        DEBUG_CODE(tree._M_dealloc_count = tree._M_alloc_count = 0);
        tree._M_head = nullptr;
        tree._M_size = 0;
        if(_M_head)
            _M_head->set_base_class(this);
        return *this;
    }

    // Recursive copy
    TEMPLATE
    void Octree<Type>::copy_tree(Node*& to, Node* node, Node* prev)
    {
        to = new_node(prev, node->_M_box);
        to->_M_objects = node->_M_objects;
        _M_size += to->_M_objects.size();
        for (int i = 0; i < 8; i++) copy_tree(to->_M_parts[i], node->_M_parts[i], to);
    }

    TEMPLATE
    Octree<Type>& Octree<Type>::clear()
    {
        if (_M_head)
            delete _M_head;
        _M_head = nullptr;
        _M_size = 0;
        return *this;
    }


    TEMPLATE
    Octree<Type>::~Octree()
    {
        if (_M_head)
        {
            delete _M_head;
        }
    }

    TEMPLATE
    typename Octree<Type>::Node* Octree<Type>::new_node(Octree<Type>::Node* prev, const AABB_3D& box)
    {
        DEBUG_CODE(_M_alloc_count++);
        Node* node = new Node;
        node->_M_prev = prev;
        node->_M_tree = this;
        node->_M_box = box;
        return node;
    }


    DEBUG_CODE(TEMPLATE std::size_t Octree<Type>::alloc_count() const { return _M_alloc_count; });
    DEBUG_CODE(TEMPLATE std::size_t Octree<Type>::dealloc_count() const { return _M_dealloc_count; });

    TEMPLATE
    std::size_t Octree<Type>::depth() const
    {
        return _M_head ? _M_head->depth() : 0;
    }

    TEMPLATE
    typename Octree<Type>::Node* Octree<Type>::get(const Octree::Index& index)
    {
        return _M_head ? _M_head->get(index) : nullptr;
    }

    TEMPLATE
    typename Octree<Type>::Node& Octree<Type>::operator[](const Octree::Index& index)
    {
        if (_M_head)
            return (*_M_head)[index];
        throw std::runtime_error("Octree: Index out of range");
    }

    TEMPLATE
    const typename Octree<Type>::Node* Octree<Type>::get(const Octree::Index& index) const
    {
        return _M_head ? _M_head->get(index) : nullptr;
    }

    TEMPLATE
    const typename Octree<Type>::Node& Octree<Type>::operator[](const Octree::Index& index) const
    {
        if (_M_head)
            return (*_M_head)[index];
        throw std::runtime_error("Octree: Index out of range");
    }

    TEMPLATE
    std::size_t Octree<Type>::size() const
    {
        return _M_size;
    }

}// namespace Engine

#undef TEMPLATE
