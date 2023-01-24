#pragma once
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <unordered_map>

namespace Engine
{

#define MAX_BONES_COUNT 2000
#define MAX_BONE_INFLUENCE 4

    struct BoneInfo {
        ObjID ID = 0;
        Matrix4f offset = Constants::identity_matrix;
    };

    using BonesMap = std::unordered_map<std::string, BoneInfo>;

    struct SkeletalVertexPart {
        int bone_ids[MAX_BONE_INFLUENCE];
        float weights[MAX_BONE_INFLUENCE];
    };

}// namespace Engine
