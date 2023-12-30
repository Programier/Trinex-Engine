#pragma once
#include <Core/engine_types.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
    template<typename ElementType>
    class Octree
    {
    public:
        struct Index {
            byte x : 1 = 0;
            byte y : 1 = 0;
            byte z : 1 = 0;

            Index(bool x, bool y, bool z) : x(x ? 1 : 0), y(y ? 1 : 0), z(z ? 1 : 0)
            {}

            Index(byte index = 0) : x((index >> 2) & 1), y((index >> 1) & 1), z(index & 1)
            {}

            Index(const Index&)            = default;
            Index& operator=(const Index&) = default;

            FORCE_INLINE byte index() const
            {
                return (x << 2) | (y << 1) | z;
            }

            FORCE_INLINE Vector3D factor() const
            {
                return {x == 1 ? 1.0f : -1.0f, y == 1 ? 1.0f : -1.0f, z == 1 ? 1.0f : -1.0f};
            }

            Index operator!() const
            {
                Index res;
                res.x = !x;
                res.y = !y;
                res.z = !z;
                return res;
            }
        };

        class Node
        {
        private:
            Node* _M_childs[8];
            AABB_3Df _M_box;

            Node()
            {
                for (int i = 0; i < 8; i++) _M_childs[i] = nullptr;
            }

            Node(const AABB_3Df& box) : Node()
            {
                _M_box = box;
            }

            Node(const Node& node)
            {
                *this = node;
            }

            Node& operator=(const Node& other)
            {
                if (this == &other)
                    return *this;

                _M_box = other._M_box;
                values = other.values;

                for (int i = 0; i < 8; i++)
                {
                    if (other._M_childs[i])
                    {
                        if (!_M_childs[i])
                        {
                            _M_childs[i] = new Octree::Node(*other._M_childs[i]);
                        }
                        else
                        {
                            (*_M_childs[i]) = *other._M_childs[i];
                        }
                    }
                    else if (_M_childs[i])
                    {
                        delete _M_childs[i];
                        _M_childs[i] = nullptr;
                    }
                }
            }

            ~Node()
            {
                for (int i = 0; i < 8; i++)
                {
                    if (_M_childs[i] != nullptr)
                    {
                        delete _M_childs[i];
                        _M_childs[i] = nullptr;
                    }
                }
            }


            FORCE_INLINE Node*& ref_child_at(Octree::Index index)
            {
                return _M_childs[index.index()];
            }

        public:
            TreeSet<ElementType> values;

            FORCE_INLINE Node* child_at(Octree::Index index) const
            {
                return _M_childs[index.index()];
            }

            FORCE_INLINE const AABB_3Df& box() const
            {
                return _M_box;
            }

            friend class Octree;
        };

    private:
        Node* _M_root_node = nullptr;
        float _M_min_size  = 1.0f;

        static FORCE_INLINE bool calc_axis_offset_index(float parent, float child)
        {
            return parent < child;
        }

        static FORCE_INLINE Octree::Index calc_child_index(const AABB_3Df& parent, const AABB_3Df& child)
        {
            Octree::Index result;
            auto parent_center = parent.center();
            auto child_center  = child.center();
            result.x           = calc_axis_offset_index(parent_center.x, child_center.x);
            result.y           = calc_axis_offset_index(parent_center.y, child_center.y);
            result.z           = calc_axis_offset_index(parent_center.z, child_center.z);
            return result;
        }

        static FORCE_INLINE bool is_child_of(const AABB_3Df& parent, const AABB_3Df& child)
        {
            return child.inside(parent) && !child.contains(parent.center());
        }

    public:
        Octree(float min_size = 1.0f) : _M_min_size(min_size)
        {
            _M_root_node = new Octree::Node(AABB_3Df({0.0f, 0.0f, 0.0f}, {min_size, min_size, min_size}));
        }

        FORCE_INLINE Node* root_node() const
        {
            return _M_root_node;
        }

        FORCE_INLINE Node* push(const AABB_3Df& box, const ElementType& element)
        {
            Node* node = find_or_create(box);
            node->values.insert(element);
            return node;
        }

        FORCE_INLINE Node* remove(const AABB_3Df& box, const ElementType& element)
        {
            Node* node = find(box);
            if (node)
            {
                node->values.erase(element);
            }

            return node;
        }

        FORCE_INLINE Node* find(const AABB_3Df& box) const
        {
            Node* node = _M_root_node;

            if (!box.inside(node->_M_box))
                return nullptr;

            while (node && is_child_of(node->_M_box, box))
            {
                node = node->child_at(calc_child_index(node->_M_box, box));
            }
            return node;
        }

        FORCE_INLINE Node* find_or_create(const AABB_3Df& box)
        {
            Node* node = _M_root_node;

            while (!box.inside(node->_M_box))
            {
                Octree::Index index = calc_child_index(box, node->_M_box);
                node                = new Octree::Node((node->_M_box * 2.0f) + (node->_M_box.size() / 2.0f) * (!index).factor());
                node->_M_childs[index.index()] = _M_root_node;
                _M_root_node                   = node;
            }

            while (node && node->_M_box.size().x / 2 >= _M_min_size && is_child_of(node->_M_box, box))
            {
                Octree::Index index = calc_child_index(node->_M_box, box);
                if (node->_M_childs[index.index()] == nullptr)
                {
                    node->_M_childs[index.index()] =
                            new Octree::Node((node->_M_box * 0.5) + (node->_M_box.size() * 0.25f) * index.factor());
                }
                node = node->_M_childs[index.index()];
            }

            return node;
        }

        ~Octree()
        {
            delete _M_root_node;
        }
    };
}// namespace Engine
