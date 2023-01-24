#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Graphics/drawable.hpp>
#include <Graphics/mesh.hpp>


class aiMesh;

namespace Engine
{
    ENGINE_EXPORT struct StaticLineVertex {
        Point3D position;
    };

    ENGINE_EXPORT class StaticLineMesh : public Mesh<StaticLineVertex>
    {
    };

    class BasicMesh;
    class Scene;

    ENGINE_EXPORT class StaticLine : public Drawable
    {
    private:
        StaticLineMesh* _M_mesh = nullptr;

        declare_instance_info_hpp(StaticLine);

    public:
        Color color = Color::Green;
        delete_copy_constructors(StaticLine);
        constructor_hpp(StaticLine);
        std::size_t render(const Matrix4f& matrix);
        StaticLineMesh* mesh();
        const StaticLineMesh* mesh() const;
        StaticLine& mesh(StaticLineMesh* mesh);

        ENGINE_EXPORT static StaticLine* load_from_assimp_mesh(const aiMesh* mesh);
        ENGINE_EXPORT static void load(const std::string& filename, Scene* scene);
    };
}// namespace Engine
