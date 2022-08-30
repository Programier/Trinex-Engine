#include <Graphics/object.hpp>
#include <assimp/scene.h>
#include <engine.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Engine
{

    static AABB_3D get_aabb(const aiScene* scene)
    {
        AABB_3D aabb;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++)
        {
            auto& current_aabb = scene->mMeshes[i]->mAABB;
            for (int j = 0; j < 3; j++)
                if (aabb.min[j] > current_aabb.mMin[j])
                    aabb.min[j] = current_aabb.mMin[j];
            for (int j = 0; j < 3; j++)
                if (aabb.max[j] < current_aabb.mMax[j])
                    aabb.max[j] = current_aabb.mMax[j];
        }
        return aabb;
    }


}// namespace Engine
