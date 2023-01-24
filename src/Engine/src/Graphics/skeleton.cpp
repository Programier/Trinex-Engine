#include <Core/assimp_helpers.hpp>
#include <Core/string_convert.hpp>
#include <Graphics/skeleton.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>

namespace Engine
{
    declare_instance_info_cpp(Skeleton);

    constructor_cpp(Skeleton)
    {}

    Bone* load_skeletal(aiNode* root_bone)
    {
        if (!root_bone)
            return nullptr;


        struct Node {
            aiNode* node = nullptr;
            Bone* bone = nullptr;
        };

        std::list<Node> _M_stack = {{root_bone, nullptr}};
        Bone* bone = nullptr;

        while (!_M_stack.empty())
        {
            Node node = _M_stack.back();
            _M_stack.pop_back();

            bone = Object::new_instance<Bone>(node.bone);
            bone->name(Strings::to_wstring(node.node->mName.data));
            bone->model(AssimpHelpers::get_matrix4(&node.node->mTransformation));

            for (decltype(node.node->mNumChildren) i = 0; i < node.node->mNumChildren; i++)
            {
                _M_stack.push_back({node.node->mChildren[i], bone});
            }
        }

        return bone->get_head_bone();
    }

    Skeleton& Skeleton::load_skeleton(const aiBone* bone)
    {
        erase();

        // Find root bone
        if (!bone->mArmature || !bone)
            return *this;

        // Find root bone
        aiNode* root_node = bone->mNode;
        if (!root_node)
            return *this;

        while (root_node->mParent != nullptr && root_node->mParent != bone->mArmature) root_node = root_node->mParent;

        // Start loading armature
        _M_root_bone = &load_skeletal(root_node)->calculate_indeces();
        return *this;
    }

    Skeleton& Skeleton::erase()
    {
        if (_M_root_bone)
        {
            _M_root_bone->mark_for_delete();
            _M_root_bone = nullptr;
        }
        return *this;
    }

    Bone* Skeleton::root_bone() const
    {
        return _M_root_bone;
    }

    Skeleton::~Skeleton()
    {
        erase();
    }
}// namespace Engine
