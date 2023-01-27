#pragma once
#include <Graphics/scene_node.hpp>

namespace Engine
{
    ENGINE_EXPORT class Scene3D : public SceneTree
    {
        declare_instance_info_hpp(Scene3D);

    public:
        delete_copy_constructors(Scene3D);
        constructor_hpp(Scene3D);

    };
}
