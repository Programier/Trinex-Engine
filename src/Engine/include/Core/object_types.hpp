#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{

    enum class ObjectType : EnumerateType
    {
        Object,
        Package,
        Animation,
        Animator,
        Channel,
        BasicFrameBuffer,
        ApiObject,
        ModelMatrix,
        Translate,
        Rotate,
        Scale,
        BasicObject,
        AnimatedObject,
        SSBO,
        BasicDrawable,
        Font,
        FrameBuffer,
        Drawable,
        StaticLine,
        BasicMesh,
        Mesh,
        OctreeBase,
        OctreeBaseNode,
        OctreeNode,
        Octree,
        Scene,
        Shader,
        Skeleton,
        Skybox,
        BasicSSBO,
        Texture,
        Texture2D,
        TextureCubeMap,
        TexturedObject,
        StaticTexturedObject,
        AnimatedTexturedObject,
        Camera,
        SceneNode,
        SceneTree,
        Scene3D,
        Application,
        Count
    };
}// namespace Engine
