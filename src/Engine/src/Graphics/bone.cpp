#include <Core/logger.hpp>
#include <Graphics/bone.hpp>
#include <Graphics/textured_object.hpp>


namespace Engine
{

    static BasicComparator<ArrayIndex> basic_comparator;

    int BoneComparator::operator()(const struct Bone* a, const struct Bone* b) const
    {
        return basic_comparator(a->index(), b->index());
    }


    Bone::Bone(Bone* parent) : _M_bones_count(1)
    {
        if (parent)
            parent->add_child(this);
        _M_parent = parent;
    }

    Bone& Bone::add_child(Bone* child)
    {
        if (child)
        {
            if (child->_M_parent)
                child->_M_parent->remove_child(child);
            _M_childs.push(child);
            child->_M_parent = this;

            Bone* node = this;
            while (node)
            {
                node->_M_bones_count += child->_M_bones_count;
                node = node->_M_parent;
            }
        }

        return *this;
    }

    Bone& Bone::remove_child(Bone* child)
    {
        if (child && _M_childs.contains(child))
        {
            _M_childs.remove(child);
            child->_M_parent = nullptr;

            Bone* node = this;
            while (node)
            {
                node->_M_bones_count -= child->_M_bones_count;
                node = node->_M_parent;
            }
        }
        return *this;
    }

    const Bone::BoneSet& Bone::childs() const
    {
        return _M_childs;
    }

    Bone* Bone::parent() const
    {
        return _M_parent;
    }

    template<typename BoneType, typename Predicate>
    static BoneType* find_bone(BoneType* bone, Predicate predicate)
    {
        if (predicate(bone))
            return bone;
        for (auto& child : bone->childs())
        {
            auto res = find_bone(child, predicate);
            if (res)
                return res;
        }
        return nullptr;
    }


    int bone_idx_comparator(const Bone* bone, ArrayIndex index)
    {
        auto idx = bone->index();
        if (idx == index)
            return 0;
        if (idx > index)
            return 1;
        return -1;
    }

    template<typename BoneType>
    static BoneType* find_bone(BoneType* bone, ArrayIndex index)
    {
        auto idx = bone->index();
        if (idx == index)
            return bone;

        if (idx > index)
            return nullptr;

        idx = bone->childs().element_index(index, bone_idx_comparator, OnNoneIndex::UseRight);

        return nullptr;
    }

    Bone* Bone::find_bone_by_name(const String& _name)
    {
        return find_bone(this, [&_name](Bone* bone) { return bone->name() == _name; });
    }

    const Bone* Bone::find_bone_by_name(const String& _name) const
    {
        return find_bone(this, [&_name](const Bone* bone) { return bone->name() == _name; });
    }

    Bone* Bone::find_bone_by_index(ArrayIndex index)
    {
        return find_bone(this, index);
    }

    const Bone* Bone::find_bone_by_index(ArrayIndex index) const
    {
        return find_bone(this, index);
    }

    ArrayIndex Bone::index() const
    {
        return _M_index;
    }

    Bone* Bone::get_head_bone()
    {
        Bone* head = this;
        while (head->_M_parent) head = head->_M_parent;
        return head;
    }

    const Bone* Bone::get_head_bone() const
    {
        const Bone* head = this;
        while (head->_M_parent) head = head->_M_parent;
        return head;
    }

    ArrayIndex Bone::recursive_calculate_indexes(ArrayIndex current_index)
    {
        _M_index = current_index;
        ++current_index;
        for (auto& ell : _M_childs) current_index = ell->recursive_calculate_indexes(current_index);
        _M_childs.sort();
        return current_index;
    }

    Bone& Bone::calculate_indeces()
    {
        get_head_bone()->recursive_calculate_indexes(0);
        return *this;
    }

    std::size_t Bone::bones_count() const
    {
        return _M_bones_count;
    }
}// namespace Engine
