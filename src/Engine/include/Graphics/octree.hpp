#pragma once

#include <Core/implement.hpp>
#include <Graphics/hitbox.hpp>
#include <array>
#include <iostream>
#include <unordered_set>


namespace Engine
{

    struct OctreeIndex {
    public:
        byte x : 1;
        byte y : 1;
        byte z : 1;

        OctreeIndex();
        OctreeIndex(byte value);

        operator byte();
    };

    class OctreeBase
    {
    protected:
        float _M_min_size = 1.f;

    public:
        implement_class_hpp(OctreeBase);
        OctreeBase(float min_size);

        void normalize_shift(Offset3D& shift);
        void generate_box(BoxHB& box);
        void generate_box(const Point3D& point, const Point3D& new_center, BoxHB& out);
        OctreeIndex index_from_normalized_shift(const Offset3D& shift);
        void generate_biggest_box(const BoxHB& box, Offset3D offset, BoxHB& out);
    };

    template<typename Type>
    class Octree final : protected OctreeBase
    {

    public:
        struct OctreeNode {
        private:
            OctreeIndex _M_index = 0;
            OctreeNode* _M_parent = nullptr;
            std::array<std::array<std::array<OctreeNode*, 2>, 2>, 2> _M_nodes;
            Octree* const _M_tree;
            BoxHB _M_box;

        public:
            std::unordered_set<Type> values;

            OctreeIndex index_at_parent() const
            {
                return _M_index;
            }

            OctreeNode* parent() const
            {
                return _M_parent;
            }

            Octree* const octree() const
            {
                return _M_tree;
            }

            const BoxHB& box() const
            {
                return _M_box;
            }

            void destroy()
            {
                if (_M_parent)
                    _M_parent->private_get(_M_index) = nullptr;

                if (this == _M_tree->_M_head)
                    _M_tree->_M_head = nullptr;

                for (byte i = 0; i < 8; i++)
                {
                    auto node = get(i);
                    if (node)
                        delete node;
                }
            }


            OctreeNode* get(const OctreeIndex& index) const
            {
                return _M_nodes[index.x][index.y][index.z];
            }

            ~OctreeNode()
            {
                destroy();
            }

            friend class Octree;

        private:
            OctreeNode(Octree* const _M_tree)
                : _M_tree(_M_tree), _M_box(Size3D(Constants::zero_vector), Size3D(Constants::zero_vector))
            {
                for (byte i = 0; i < 8; i++) private_get(i) = nullptr;
            }

            OctreeNode*& private_get(const OctreeIndex& index)
            {
                return _M_nodes[index.x][index.y][index.z];
            }

            OctreeNode(const OctreeNode& node)
            {
                *this = node;
            }

            OctreeNode* copy(Octree* const tree)
            {
                OctreeNode* node = new OctreeNode(tree);
                node->_M_index = _M_index;
                node->_M_box = _M_box;

                for (byte i = 0; i < 8; i++)
                {
                    OctreeNode* part = get(i);
                    if (part)
                    {
                        node->private_get(i) = part->copy(tree);
                    }
                }

                return node;
            }
        };

    private:
        OctreeNode* _M_head = nullptr;

        template<typename Access>
        Access* basic_find(BoxHB box)
        {
            generate_box(box);

            OctreeNode* _M_node = _M_head;
            const Size3D& _M_box_half_size = box.half_size();
            const Point3D& _M_box_center = box.center();

            while (_M_node && _M_node->_M_box.contains(box))
            {
                const auto _M_node_center = _M_node->_M_box.center();
                Offset3D _M_center_offset = _M_box_center - _M_node_center;
                Offset3D _M_abs_center_offset = glm::abs(_M_center_offset) - Constants::min_positive_vector;

                if (_M_abs_center_offset.x < _M_box_half_size.x || _M_abs_center_offset.y < _M_box_half_size.y ||
                    _M_abs_center_offset.z < _M_box_half_size.z)
                    return _M_node;

                normalize_shift(_M_center_offset);

                _M_node = _M_node->private_get(index_from_normalized_shift(_M_center_offset));
            }

            return nullptr;
        }

        OctreeNode* private_find(BoxHB box)
        {
            generate_box(box);

            OctreeNode* _M_node = _M_head;
            const Size3D& _M_box_half_size = box.half_size();
            const Point3D& _M_box_center = box.center();

           // std::clog << _M_box_center << "\t" << _M_box_center << std::endl;

            // Find down

        find_down:

            while (_M_node->_M_box.contains(box))
            {
                //std::clog << _M_node->box.half_size() << std::endl;
                const auto _M_node_center = _M_node->_M_box.center();
                Offset3D _M_center_offset = _M_box_center - _M_node_center;

                Offset3D _M_abs_center_offset = glm::abs(_M_center_offset) - Constants::min_positive_vector;

                if (_M_abs_center_offset.x < _M_box_half_size.x || _M_abs_center_offset.y < _M_box_half_size.y ||
                    _M_abs_center_offset.z < _M_box_half_size.z)
                    return _M_node;

                normalize_shift(_M_center_offset);

                OctreeIndex index = index_from_normalized_shift(_M_center_offset);
                OctreeNode*& _M_new_node = _M_node->private_get(index);
                if (!_M_new_node)
                {
                    _M_new_node = new OctreeNode(this);
                    _M_new_node->_M_parent = _M_node;
                    _M_new_node->_M_index = index;
                    const auto tmp = _M_node->_M_box.half_size() * _M_center_offset;
                    generate_box(_M_node_center, _M_node_center + tmp * 0.5f, _M_new_node->_M_box);
                }

                _M_node = _M_new_node;
            }


            // Find up

            do
            {
                auto _M_node_center = _M_node->_M_box.center();
                Offset3D _M_center_offset = _M_box_center - _M_node_center;
                normalize_shift(_M_center_offset);

                if (_M_node->_M_parent == nullptr)
                {
                    _M_head = new OctreeNode(this);

                    generate_biggest_box(_M_node->_M_box, _M_center_offset, _M_head->_M_box);

                    OctreeIndex index = index_from_normalized_shift(-_M_center_offset);
                    _M_head->private_get(index) = _M_node;
                    _M_node->_M_index = index;

                    _M_node->_M_parent = _M_head;
                    _M_node = _M_head;
                }
                else
                {
                    throw std::runtime_error("Octree: Floating point error!");
                }
            } while (!_M_node->_M_box.contains(box));

            goto find_down;
        }

    public:
        Octree(float min_size = 1.f) : OctreeBase(min_size)
        {}

        Octree(const Octree& octree)
        {
            *this = octree;
        }

        Octree(Octree&& octree)
        {
            *this = std::move(octree);
        }

        Octree& operator=(const Octree& base)
        {
            clear();
            _M_head = base._M_head->copy(this);
            return *this;
        }

        Octree& operator=(Octree&& base)
        {
            clear();
            _M_head = base._M_head;
            base._M_head = nullptr;
            return *this;
        }

        Octree& push(const Type& value, const BoxHB& box)
        {
            if (!_M_head)
            {
                _M_head = new OctreeNode(this);
                _M_head->_M_box = box;

                generate_box(_M_head->_M_box);
            }

            OctreeNode* node = private_find(box);
            if (!node)
                std::runtime_error("Octree error!");
            node->values.insert(value);

            if (node->_M_parent == nullptr)
                _M_head = node;
            return *this;
        }

        OctreeNode* find(const BoxHB& box)
        {
            return basic_find<OctreeNode>(box);
        }

        const OctreeNode* find(const BoxHB& box) const
        {
            return basic_find<const OctreeNode>(box);
        }

        OctreeNode* head() const
        {
            return _M_head;
        }

        Octree& clear()
        {
            if (_M_head)
                delete _M_head;
            return *this;
        }

        BoxHB box() const
        {
            if (_M_head)
                return _M_head->box();
            return BoxHB();
        }


        ~Octree()
        {
            clear();
        }

        friend struct OctreeNode;
    };


}// namespace Engine
