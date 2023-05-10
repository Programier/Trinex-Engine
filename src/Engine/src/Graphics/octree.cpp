#include <Core/class.hpp>
#include <Graphics/frustum.hpp>
#include <Graphics/octree.hpp>

namespace Engine
{
    register_class(Engine::OctreeBase, Engine::Object);
    OctreeBase::OctreeBase(float min_size)
    {
        this->_M_min_size = glm::abs(min_size);
    }


    OctreeIndex::OctreeIndex(byte value)
    {
        z = value & 1;
        y = (value >> 1) & 1;
        x = (value >> 2) & 1;
    }

    OctreeIndex::OctreeIndex() : OctreeIndex(0)
    {}

    OctreeIndex::operator byte()
    {
        byte result = (((x << 1) + y) << 1) + z;
        return result;
    }


    void OctreeBase::normalize_shift(Offset3D& shift)
    {
        shift.x = shift.x < 0 ? -1.f : 1.f;
        shift.y = shift.y < 0 ? -1.f : 1.f;
        shift.z = shift.z < 0 ? -1.f : 1.f;
    }


    void OctreeBase::generate_box(BoxHB& box)
    {
        auto half = box.half_size();
        float m   = glm::max(glm::max(glm::max(half.x, half.y), half.z), _M_min_size);
        box.aabb(AABB_3D{box.center() - Size3D(m), box.center() + Size3D(m)});
    }

    OctreeIndex OctreeBase::index_from_normalized_shift(const Offset3D& offset)
    {
        OctreeIndex index(0);

        index.x = offset.x >= 0.f;
        index.y = offset.y >= 0.f;
        index.z = offset.z >= 0.f;


        return index;
    }

    void OctreeBase::generate_box(const Point3D& point, const Point3D& new_center, BoxHB& out)
    {
        const auto half_size = point - new_center;
        out.aabb(new_center - half_size, new_center + half_size);
    }


#define check_offset_coord(coord)                                                                                      \
    if (offset.coord == 0.f)                                                                                           \
    offset.coord = 1.f

    void OctreeBase::generate_biggest_box(const BoxHB& box, Offset3D offset, BoxHB& out)
    {
        auto new_half_size = Size3D(glm::max(box.half_size().x * 2, _M_min_size));

        check_offset_coord(x);
        check_offset_coord(y);
        check_offset_coord(z);

        auto new_center = box.center() + offset * box.half_size();

        out.aabb(AABB_3D{new_center - new_half_size, new_center + new_half_size});
    }


    register_class(Engine::OctreeBaseNode, Engine::Object, _M_box(Constants::zero_vector, Constants::zero_vector));
    OctreeBaseNode::OctreeBaseNode()
    {}

    OctreeBaseNode* OctreeBaseNode::get(const OctreeIndex& index) const
    {
        throw EngineException("OctreeBaseNode: Pure virtual call!");
    }

    OctreeIndex OctreeBaseNode::index_at_parent() const
    {
        return _M_index;
    }

    const BoxHB& OctreeBaseNode::box() const
    {
        return _M_box;
    }

    std::size_t OctreeBaseNode::render()
    {
        throw std::runtime_error(not_implemented);
    }

    OctreeBaseNode::~OctreeBaseNode()
    {}

}// namespace Engine
