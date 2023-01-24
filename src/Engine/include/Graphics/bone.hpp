#pragma once
#include <Core/export.hpp>
#include <Core/object.hpp>
#include <Graphics/basic_object.hpp>
#include <TemplateFunctional/array_containers.hpp>
#include <vector>

namespace Engine
{
    struct VertexWeight {
        ArrayIndex vertex_index;
        float weight;
    };

    class AnimatedObject;


    using VertexWeightArray = std::vector<VertexWeight>;

    struct BoneComparator
    {
        int operator()(const struct Bone* a, const struct Bone* b) const;
    };

    ENGINE_EXPORT struct Bone final : public BasicObject<Translate, Rotate, Scale> {
    public:
        using BoneSet = SortedDynamicSet<Bone*, BoneComparator>;

    private:
        Bone* _M_parent = nullptr;
        BoneSet _M_childs;
        ArrayIndex _M_index;
        std::size_t _M_bones_count;

        ArrayIndex recursive_calculate_indexes(ArrayIndex index);

    public:
        Matrix4f offset_matrix = Constants::identity_matrix;

    public:
        Bone(Bone* parent = nullptr);
        Bone& add_child(Bone* child);
        Bone& remove_child(Bone* child);
        const BoneSet& childs() const;
        Bone* parent() const;
        Bone* find_bone_by_name(const String& name);
        const Bone* find_bone_by_name(const String& name) const;
        Bone* find_bone_by_index(ArrayIndex index);
        const Bone* find_bone_by_index(ArrayIndex index) const;
        ArrayIndex index() const;
        Bone& calculate_indeces();
        std::size_t bones_count() const;
        Bone* get_head_bone();
        const Bone* get_head_bone() const;
    public:
        VertexWeightArray weights;
    };

    using Bones = std::vector<Bone>;
}// namespace Engine
