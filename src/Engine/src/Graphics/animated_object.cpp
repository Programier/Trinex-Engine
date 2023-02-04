#include <Core/assimp_helpers.hpp>
#include <Core/check.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/animated_object.hpp>
#include <Graphics/scene.hpp>
#include <Graphics/skeleton.hpp>
#include <assimp/scene.h>


namespace Engine
{
    Bone* AnimatedObject::root_bone()
    {
        return _M_root;
    }

    const Bone* AnimatedObject::root_bone() const
    {
        return _M_root;
    }


    static Bone* static_load_skeletal(const aiNode* node, const std::unordered_map<std::string, const aiBone*>& bones)
    {
        if (!node)
            return nullptr;

        Bone* bone = nullptr;

        if (bones.contains(node->mName.data))
        {
            bone = Object::new_instance<Bone>();

            const aiBone* ai_bone = bones.at(node->mName.data);
            check(ai_bone);
            bone->name(ai_bone->mName.data);
            bone->offset_matrix = AssimpHelpers::get_matrix4(&ai_bone->mOffsetMatrix);

            bone->weights.resize(ai_bone->mNumWeights);
            for (decltype(ai_bone->mNumWeights) i = 0; i < ai_bone->mNumWeights; i++)
            {
                auto& weight = ai_bone->mWeights[i];
                bone->weights[i].weight = weight.mWeight;
                bone->weights[i].vertex_index = weight.mVertexId;
            }
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            auto child = static_load_skeletal(node->mChildren[i], bones);
            bone = bone ? &bone->add_child(child) : child;
        }

        return bone;
    }

    static void mark_bones_for_delete(Bone* bone)
    {
        if (bone == nullptr)
            return;

        std::list<Bone*> stack = {bone};
        while (!stack.empty())
        {
            auto node = stack.back();
            stack.pop_back();
            node->mark_for_delete();

            if (node->childs().size() > 2)
            {
                int a = 0;
                a++;
            }
            for (auto child : node->childs()) stack.push_back(child);
        }
    }

    declare_instance_info_cpp(AnimatedObject);

    constructor_cpp(AnimatedObject)
    {}

    AnimatedObject& AnimatedObject::update_animation_matrices()
    {
        if (!_M_root)
            return *this;

        struct Node {
            Matrix4f parent;
            Bone* bone;
        };

        std::list<Node> stack = {{find_scene_node(Scene::get_active_scene())->global_matrix(), _M_root}};

        while (!stack.empty())
        {
            auto node = stack.back();
            stack.pop_back();

            const auto new_model = node.parent * node.bone->model();
            _M_ssbo.data[node.bone->index()] = new_model;


            for (auto child : node.bone->childs()) stack.push_back({new_model, child});
        }

        _M_ssbo.update_data(0, _M_ssbo.size());
        return *this;
    }

    AnimatedObject& AnimatedObject::load_skeletal(const aiScene* scene, const aiMesh* mesh)
    {
        check(scene);
        mark_bones_for_delete(_M_root);
        std::unordered_map<std::string, const aiBone*> bones;
        bones.reserve(mesh->mNumBones);

        for (decltype(mesh->mNumBones) i = 0; i < mesh->mNumBones; i++)
            bones[mesh->mBones[i]->mName.data] = mesh->mBones[i];

        if (!bones.empty())
        {
            _M_skeleton = new_instance<Skeleton>();
            _M_skeleton->load_skeleton((*bones.begin()).second);
        }

        _M_root = static_load_skeletal(scene->mRootNode, bones);
        if (_M_root)
        {
            _M_root->calculate_indeces();

            _M_ssbo.destroy();
            _M_ssbo.data.clear();
            _M_ssbo.usage = BufferUsage::DYNAMIC_COPY;

            _M_ssbo.data.resize(_M_root->bones_count(), Constants::identity_matrix);
            _M_ssbo.gen().set_data();
        }

        _M_animations.clear();
        for (decltype(scene->mNumAnimations) i = 0; i < scene->mNumAnimations; i++)
        {
            auto ai_animation = scene->mAnimations[i];
            std::list<const aiNodeAnim*> ai_nodes;

            for (decltype(ai_animation->mNumChannels) j = 0; j < ai_animation->mNumChannels; j++)
            {
                if (bones.contains(ai_animation->mChannels[j]->mNodeName.data))
                    ai_nodes.push_back(ai_animation->mChannels[j]);
            }

            if (!ai_nodes.empty())
                _M_animations.push_back(&Object::new_instance<Animation>(this)->load(ai_animation, ai_nodes, _M_root));
        }
        return *this;
    }

    AnimatedObject& AnimatedObject::set_weights_to_vertices()
    {
        if (!_M_root)
            return *this;

        std::list<Bone*> stack = {_M_root};

        while (!stack.empty())
        {
            Bone* bone = stack.back();
            stack.pop_back();

            int index = static_cast<int>(bone->index());
            for (auto& weight : bone->weights)
            {
                VertexWeights* v_weight = get_vertex_weights(weight.vertex_index);

                if (!v_weight)
                    continue;

                int i = 0;
                while (i < MAX_BONES_PER_VERTEX && v_weight->bones_id[i] != -1) ++i;
                if (i == MAX_BONES_PER_VERTEX)
                {
                    logger->log("Animated object: Max bones per vertex is %d! Skipping weight!\n",
                                MAX_BONES_PER_VERTEX);
                    continue;
                }

                v_weight->bones_id[i] = index;
                v_weight->weights[i] = weight.weight;
            }

            for (auto& ell : bone->childs()) stack.push_back(ell);
        }
        return *this;
    }

    const std::vector<Animation*> AnimatedObject::animations() const
    {
        return _M_animations;
    }

    const std::vector<Matrix4f>& AnimatedObject::animation_martices() const
    {
        return _M_ssbo.data;
    }

    AnimatedObject::~AnimatedObject()
    {
        mark_bones_for_delete(_M_root);
        for (auto anim : _M_animations)
        {
            anim->mark_for_delete();
        }
    }


    Skeleton* AnimatedObject::skeleton() const
    {
        return _M_skeleton;
    }

    AnimatedObject& AnimatedObject::skeleton(Skeleton* skeleton)
    {
        _M_skeleton = skeleton;
        return *this;
    }
}// namespace Engine
