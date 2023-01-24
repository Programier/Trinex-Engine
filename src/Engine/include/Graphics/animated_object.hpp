#pragma once
#include <Core/export.hpp>
#include <Graphics/animation.hpp>
#include <Graphics/bone.hpp>
#include <Graphics/drawable.hpp>
#include <Graphics/ssbo.hpp>


class aiScene;
class aiMesh;
#define MAX_BONES_PER_VERTEX 4

namespace Engine
{

    class Skeleton;
    struct VertexWeights {
        IntVector4D bones_id = -Constants::int_identity_vector;
        Vector4D weights = Constants::identity_vector;
    };


    ENGINE_EXPORT class AnimatedObject : public virtual Drawable
    {
    private:
        Bone* _M_root = nullptr;
        std::vector<Animation*> _M_animations;
        virtual VertexWeights* get_vertex_weights(ArrayIndex index) = 0;


        Skeleton* _M_skeleton = nullptr;


    protected:
        constructor_hpp(AnimatedObject);
        SSBO<Matrix4f> _M_ssbo;

        declare_instance_info_hpp(AnimatedObject);

    public:
        delete_copy_constructors(AnimatedObject);

        AnimatedObject& update_animation_matrices();
        AnimatedObject& load_skeletal(const aiScene* scene, const aiMesh* mesh);
        AnimatedObject& set_weights_to_vertices();
        const std::vector<Animation*> animations() const;
        const std::vector<Matrix4f>& animation_martices() const;
        Bone* root_bone();
        const Bone* root_bone() const;

        Skeleton* skeleton() const;
        AnimatedObject& skeleton(Skeleton* skeleton);


        ~AnimatedObject();
    };
}// namespace Engine
