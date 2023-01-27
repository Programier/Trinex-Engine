#pragma once

#include <Core/export.hpp>
#include <Graphics/animated_object.hpp>
#include <Graphics/bone.hpp>
#include <Graphics/drawable.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/ssbo.hpp>


namespace Engine
{


    class Texture2D;
    class Scene;

    struct StaticTexturedObjectVertex {
        Vector3D position = Constants::zero_vector;
        Vector2D texture_coords = Constants::zero_vector;
        Vector3D normal = Constants::zero_vector;
    };

    struct StaticTexturedObjectMesh : public Mesh<StaticTexturedObjectVertex> {
    };

    CLASS TexturedObject : public virtual Drawable
    {
    protected:
        Texture2D* _M_diffuse_texture = nullptr;

        declare_instance_info_hpp(TexturedObject);

    public:
        constructor_hpp(TexturedObject);
        delete_copy_constructors(TexturedObject);
        Texture2D* diffuse_texture();
        const Texture2D* diffuse_texture() const;
        TexturedObject& diffuse_texture(Texture2D * texture);
    };

    class ENGINE_EXPORT StaticTexturedObject : public TexturedObject
    {
    private:
        StaticTexturedObjectMesh* _M_mesh = nullptr;

        declare_instance_info_hpp(StaticTexturedObject);

    public:
        constructor_hpp(StaticTexturedObject);
        delete_copy_constructors(StaticTexturedObject);

        std::size_t render(const Matrix4f& matrix) override;
        StaticTexturedObjectMesh* mesh();
        const StaticTexturedObjectMesh* mesh() const;
        StaticTexturedObject& mesh(StaticTexturedObjectMesh* mesh);
        static ENGINE_EXPORT void load(const String& filename, Scene* scene);
    };


    ////////////////// ANIMATED OBJECT //////////////////

    struct AnimatedTexturedObjectVertex {
        Vector3D position = Constants::zero_vector;
        Vector2D texture_coords = Constants::zero_vector;
        Vector3D normal = Constants::zero_vector;

        VertexWeights vertex_weights;
    };

    ENGINE_EXPORT struct AnimatedTexturedObjectMesh : public Mesh<AnimatedTexturedObjectVertex> {
        void clear_bones_info();
    };

    ENGINE_EXPORT class AnimatedTexturedObject : public TexturedObject, public AnimatedObject
    {
    private:
        AnimatedTexturedObjectMesh* _M_mesh = nullptr;

        declare_instance_info_hpp(AnimatedTexturedObject);

    public:
        constructor_hpp(AnimatedTexturedObject);
        delete_copy_constructors(AnimatedTexturedObject);

        VertexWeights* get_vertex_weights(ArrayIndex index) override;
        std::size_t render(const Matrix4f& matrix) override;
        AnimatedTexturedObjectMesh* mesh();
        const AnimatedTexturedObjectMesh* mesh() const;
        AnimatedTexturedObject& mesh(AnimatedTexturedObjectMesh* mesh);
        //Skeleton* skeleton() const;

        static ENGINE_EXPORT void load(const String& filename, Scene* scene);
    };
}// namespace Engine
