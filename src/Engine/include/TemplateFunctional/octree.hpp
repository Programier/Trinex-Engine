#pragma once

#include <Core/engine.hpp>
#include <Core/engine_types.hpp>
#include <iostream>
#include <list>
#include <unordered_set>

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

            Index() = default;
            Index(bool x, bool y, bool z) : x(x), y(y), z(z)
            {}

            Index(const Index&) = default;
            Index& operator=(const Index&) = default;

            Index(int index)
            {
                x = cast(bool, index& Octree::x_mask);
                y = cast(bool, index& Octree::y_mask);
                z = cast(bool, index& Octree::z_mask);
            }

            operator int() const
            {
                int _x = cast(int, x);
                int _y = cast(int, y);
                int _z = cast(int, z);

                return _x << 2 | _y << 1 | _z;
            }

            bool operator[](int index) const
            {
                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    default:
                        return false;
                }
            }
        };


        template<typename Access>
        class Iterator
        {
        protected:
            Access* _M_current = nullptr;
            Access* _M_root = nullptr;
            std::vector<Access*> _M_stack;
            bool _M_is_end = false;

            Iterator(Access* root, bool is_end = false)
            {
                _M_is_end = is_end;
                _M_root = _M_current = root;
            }

        public:
#if __cplusplus >= 201703L
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = Access;
            using pointer = value_type*;
            using reference = value_type&;
#endif


            Iterator(const Iterator&) = default;
            Iterator(Iterator&&) = default;
            Iterator& operator=(const Iterator&) = default;
            Iterator& operator=(Iterator&&) = default;
            Iterator& operator++()
            {
                for (byte i = 0; i < Octree::xyz_mask; ++i)
                {
                    pointer ptr = _M_current->get(i);
                    if (ptr)
                        _M_stack.push_back(ptr);
                }

                if (!_M_stack.empty())
                {
                    _M_current = _M_stack.back();
                    _M_stack.pop_back();
                }
                else
                {
                    _M_current = _M_root;
                    _M_is_end = !_M_is_end;
                }

                return *this;
            }


            bool operator==(const Iterator& it) const
            {
                return _M_current == it._M_current && it._M_is_end == _M_is_end;
            }

            bool operator!=(const Iterator& it) const
            {
                return !(*this == it);
            }

            reference operator*() const
            {
                return *_M_current;
            }

            pointer base() const
            {
                return _M_current;
            }

            Iterator operator++(int)
            {
                Iterator prev = *this;
                ++(*this);
                return prev;
            }

            friend class Octree;
        };

        using const_iterator = Iterator<const Octree<Type>>;
        using basic_iterator = Iterator<Octree<Type>>;

    private:
        static const byte x_mask = 1 << 2;
        static const byte y_mask = 1 << 1;
        static const byte z_mask = 1;
        static const byte xyz_mask = (x_mask | y_mask | z_mask) + 1;
        static Octree default_value;

        Octree* _M_parent = nullptr;
        Index _M_parent_index;
        Octree* _M_parts[2][2][2];

        std::size_t _M_size = 0;
        std::size_t _M_nodes = 0;
        std::size_t _M_depth = 0;

        AABB_3D _M_aabb;
        std::unordered_set<Type> _M_objects;

        Size3D _M_min_sizes = {0.1f, 0.1f, 0.1f};
        Size3D _M_min_half_sizes = {0.05f, 0.05f, 0.05f};


        void check_box(AABB_3D& box) const
        {
            box.max = glm::max(box.max, box.min);
            box.min = glm::min(box.max, box.min);

            Point3D center = (box.max + box.min) / 2.f;

            for (int i = 0; i < 3; i++)
            {
                if(box.max[i] - center[i] < _M_min_half_sizes[i])
                {
                    box.max[i] = center[i] + _M_min_half_sizes[i];
                }

                if(center[i] - box.min[i] < _M_min_half_sizes[i])
                {
                    box.min[i] = center[i] - _M_min_half_sizes[i];
                }
            }
        }

        Octree*& get(int index)
        {
            int indexes[3] = {0, 0, 0};
            for (int i = 0; i < 3; i++)
            {
                indexes[2 - i] = index % 2;
                index >>= 1;
            }

            return _M_parts[indexes[0]][indexes[1]][indexes[2]];
        }

        const Octree* get(int index) const
        {
            int indexes[3] = {0, 0, 0};
            for (int i = 0; i < 3; i++)
            {
                indexes[2 - i] = index % 2;
                index >>= 1;
            }

            return _M_parts[indexes[0]][indexes[1]][indexes[2]];
        }

        AABB_3D get_box(const Size3D& new_center, const Size3D& center)
        {
            Size3D double_center = new_center * 2.f;
            AABB_3D new_box = _M_aabb;

            for (int i = 0; i < 3; i++)
            {
                if (new_center[i] - center[i] > 0)
                    new_box.max[i] = double_center[i] - new_box.min[i];
                else
                    new_box.min[i] = double_center[i] - new_box.max[i];
            }

            check_box(new_box);
            return new_box;
        }

        Octree* create_new_head(const Size3D& new_center, const Size3D& center)
        {
            // This object is head

            AABB_3D new_box = get_box(new_center, center);

            // Allocate memory
            Octree* new_node = new Octree();
            *new_node = std::move(*this);
            _M_aabb = new_box;
            this->push_node(new_node);
            return this;
        }

        template<typename Access>
        Access* basic_find_by_box(AABB_3D box) const
        {
            check_box(box);

            Access* this_object = const_cast<Access*>(this);
            Size3D box_half_sizes = (box.max - box.min) / 2.f;
            Size3D box_center = box.max - box_half_sizes;

            while (this_object)
            {
                Size3D center = (this_object->_M_aabb.min + this_object->_M_aabb.max) / 2.f;
                Size3D current_half_size = this_object->_M_aabb.max - center;
                Size3D center_offset = box_center - center;

                bool need_continue = false;

                for (int i = 0; i < 3; i++)
                {
                    if (glm::abs(center_offset[i]) + box_half_sizes[i] > current_half_size[i])
                    {
                        need_continue = true;
                        if (this_object->_M_parent)
                            this_object = this_object->_M_parent;
                        else
                            return nullptr;
                        break;
                    }
                }

                if (need_continue)
                    continue;

                // Now we know, that box is inside this_object

                for (int i = 0; i < 3; i++)
                {
                    if (glm::abs(center_offset[i]) < box_half_sizes[i])
                    {
                        return this_object;
                    }
                }

                // Get subbox inside this box
                this_object = this_object->get(Index(center_offset.x > 0, center_offset.y > 0, center_offset.z > 0));
            }

            return this_object;
        }

        Octree* find_by_box_private(AABB_3D box, bool create = false)
        {
            check_box(box);
            Octree* this_object = this;
            Size3D box_half_sizes = (box.max - box.min) / 2.f;
            Size3D box_center = box.max - box_half_sizes;

            while (this_object)
            {
                Size3D center = (this_object->_M_aabb.min + this_object->_M_aabb.max) / 2.f;
                Size3D current_half_size = this_object->_M_aabb.max - center;
                Size3D center_offset = box_center - center;

                bool need_continue = false;
                bool need_create = false;
                Size3D direction = Constants::zero_vector;

                for (int i = 0; i < 3; i++)
                {
                    if (glm::abs(center_offset[i]) + box_half_sizes[i] > current_half_size[i])
                    {
                        need_continue = true;
                        if (this_object->_M_parent)
                        {
                            this_object = this_object->_M_parent;
                        }
                        else if (create)
                        {
                            // Generate direction
                            need_create = true;
                            direction[i] = center_offset[i] > 0 ? 1.f : -1.f;
                        }
                        else
                            return nullptr;
                    }
                }

                if (need_continue)
                {
                    if (need_create && create)
                        this_object = create_new_head(center + current_half_size * direction, center);
                    continue;
                }

                // Now we know, that box is inside this_object

                for (int i = 0; i < 3; i++)
                {
                    if (glm::abs(center_offset[i]) < box_half_sizes[i])
                    {
                        return this_object;
                    }
                }

                // Get subbox inside this box
                Index index(center_offset.x > 0, center_offset.y > 0, center_offset.z > 0);
                Octree*& tmp = this_object->get(index);
                if (tmp == nullptr && create)
                {
                    AABB_3D tmp_aabb;
                    tmp_aabb.min = center;

                    for (int i = 0; i < 3; i++)
                    {
                        float h = current_half_size[i];
                        if (h / 2.f < _M_min_sizes[i])
                        {
                            return this_object;
                        }

                        float k = index[i] ? 1.f : -1.f;
                        float m = center[i] + h * 0.5f * k;
                        m *= 2;
                        tmp_aabb.max[i] = m - tmp_aabb.min[i];
                    }

                    tmp = new Octree(this_object, index);
                    tmp->_M_aabb = tmp_aabb;

                    check_box(tmp->_M_aabb);
                }

                this_object = tmp;
            }

            return this_object;
        }


        Octree& push_node(Octree* node)
        {
            Octree* new_node = find_by_box_private(node->_M_aabb, true);
            if (new_node)
            {
                Octree*& part = new_node->_M_parent->get(new_node->_M_parent_index);
                *new_node = std::move(*node);
                part = new_node;
                delete node;
            }
            return *this;
        }

        Octree(Octree* parent, Index index) : Octree()
        {
            _M_parent = parent;
            _M_parent_index = index;
        }


    public:
        Octree()
        {
            for (byte i = 0; i < xyz_mask; ++i)
            {
                get(i) = nullptr;
            }
        }


        Octree(const Octree& tree) : Octree()
        {
            *this = tree;
        }

        Octree(Octree&& tree)
        {
            *this = std::move(tree);
        }

        Octree& recalculate_data()
        {
            _M_depth = 0;
            _M_size = _M_objects.size();
            _M_nodes = 0;

            for (byte i = 0; i < xyz_mask; i++)
            {
                Octree* ptr = get(i);
                if (ptr)
                {
                    ++_M_nodes;
                    ptr->recalculate_data();
                    _M_depth = std::max(_M_depth, ptr->_M_depth);
                    _M_size += ptr->_M_size;
                }
            }

            ++_M_depth;
            return *this;
        }


        Octree& operator=(const Octree& tree)
        {
            if (this == &tree)
                return *this;

            clear();

            _M_aabb = tree._M_aabb;
            _M_objects = tree._M_objects;

            for (byte i = 0; i < xyz_mask; ++i)
            {
                const Octree* ptr = tree.get(i);
                if (ptr)
                {
                    get(i) = new Octree(*ptr);
                    Octree* _ptr = get(i);
                    _ptr->_M_parent = this;
                    _ptr->_M_parent_index = Index(i);
                }
            }

            return *this;
        }

        Octree& operator=(Octree&& tree)
        {
            if (this == &tree)
                return *this;

            clear();

            _M_aabb = std::move(tree._M_aabb);
            tree._M_aabb = AABB_3D();
            _M_objects = std::move(tree._M_objects);
            tree._M_objects.clear();

            for (byte i = 0; i < xyz_mask; ++i)
            {
                Octree*& ptr = tree.get(i);
                if (ptr)
                {
                    ptr->_M_parent = this;
                    get(i) = ptr;
                    ptr = nullptr;
                }
            }

            if (tree._M_parent)
            {
                int i = cast(int, tree._M_parent_index);
                tree.get(i) = this;
            }

            return *this;
        }


        Octree& push(const Type& value, const AABB_3D& box)
        {
            Octree* ptr = nullptr;
            if (_M_size == 0)
            {
                _M_size++;
                _M_aabb = box;
                check_box(_M_aabb);
                ptr = this;
            }
            else
                ptr = find_by_box_private(box, true);
            ptr->_M_objects.insert(value);

            _M_size++;
            return *this;
        }

        Octree& remove(const Type& value, const AABB_3D& box)
        {
            Octree* node = find(box, false);
            if (node)
                node->_M_objects.erase(value);
            return *this;
        }

        Octree* find(const AABB_3D& box)
        {
            return basic_find_by_box<Octree>(box);
        }

        const Octree* find(const AABB_3D& box) const
        {
            return basic_find_by_box<const Octree>(box);
        }

        Octree& clear()
        {
            for (byte i = 0; i < xyz_mask; ++i)
            {
                auto& ptr = get(i);
                if (ptr)
                    delete ptr;
                ptr = nullptr;
            }

            if (_M_parent)
            {
                int index = static_cast<int>(_M_parent_index);
                _M_parent->get(index) = nullptr;
            }
            return *this;
        }

        const std::unordered_set<Type>& objects() const
        {
            return _M_objects;
        }

        std::unordered_set<Type>& objects()
        {
            return _M_objects;
        }

        const AABB_3D& aabb() const
        {
            return _M_aabb;
        }

        const std::size_t& depth() const
        {
            return _M_depth;
        }

        Octree& recalculate_size() const
        {
            throw not_implemented;
            return *this;
        }

        Octree& operator[](int index)
        {
            Octree* ptr = get(index);
            if (ptr)
                return *ptr;
            return default_value;
        }

        const Octree& operator[](int index) const
        {
            Octree* ptr = get(index);
            if (ptr)
                return *ptr;
            return default_value;
        }

        bool has_parent()
        {
            return _M_parent != nullptr;
        }

        const Octree& parent()
        {
            return *_M_parent;
        }

        const Size3D& min_sizes() const
        {
            return _M_min_sizes;
        }

        Octree& min_sizes(const Size3D& sizes)
        {
            _M_min_sizes = sizes;
        }

        const std::size_t& size() const
        {
            return _M_size;
        }

        const std::size_t& nodes_count() const
        {
            return _M_nodes;
        }

        basic_iterator begin()
        {
            return basic_iterator(this);
        }

        basic_iterator end()
        {
            return basic_iterator(this, true);
        }

        const_iterator begin() const
        {
            return const_iterator(this);
        }

        const_iterator end() const
        {
            return const_iterator(this, true);
        }

        bool empty() const
        {
            for (const Octree& ell : *this)
            {
                if (!ell._M_objects.empty())
                    return false;
            }
            return true;
        }

        ~Octree()
        {
            clear();
        }
    };
}// namespace Engine


#undef TEMPLATE
