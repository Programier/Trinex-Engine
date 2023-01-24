#pragma once
#include <Core/object.hpp>
#include <Core/export.hpp>
#include <Graphics/bone.hpp>
#include <Core/implement.hpp>


class aiNode;
class aiBone;


namespace Engine
{
    class SceneNode;

    ENGINE_EXPORT class Skeleton : public Object
    {
    private:
        Bone* _M_root_bone = nullptr;

        declare_instance_info_hpp(Skeleton);
    public:
        delete_copy_constructors(Skeleton);
        constructor_hpp(Skeleton);
        Skeleton& load_skeleton(const aiBone* bone);
        Skeleton& erase();
        Bone* root_bone() const;
        ~Skeleton();
    };
}
