#include <Core/assimp_helpers.hpp>
#include <Core/check.hpp>
#include <Core/logger.hpp>
#include <Core/string_convert.hpp>
#include <Core/string_format.hpp>
#include <Graphics/assimp.hpp>
#include <Graphics/line.hpp>
#include <Graphics/resources.hpp>
#include <Graphics/scene.hpp>
#include <Graphics/shader_system.hpp>
#include <assimp/scene.h>
#include <stack>


namespace Engine
{
    declare_instance_info_cpp(StaticLine);
    constructor_cpp(StaticLine)
    {}

    std::size_t StaticLine::render(const Matrix4f& matrix)
    {
        if (!_M_mesh)
            return 0;

        auto scene = Scene::get_active_scene();
        check_with_message(scene, "No active scene found!");

        auto camera = scene->active_camera();
        check_with_message(camera, "No active camera found!");

        namespace sh = ShaderSystem::Line;
        sh::shader.use().set(sh::model, matrix * model()).set(sh::projview, camera->projview()).set(sh::color, color);

        _M_mesh->draw(Primitive::LINE);
        return 1;
    }


    StaticLineMesh* StaticLine::mesh()
    {
        return _M_mesh;
    }

    const StaticLineMesh* StaticLine::mesh() const
    {
        return _M_mesh;
    }

    StaticLine& StaticLine::mesh(StaticLineMesh* mesh)
    {
        _M_mesh = mesh;
        return *this;
    }

    ENGINE_EXPORT StaticLine* StaticLine::load_from_assimp_mesh(const aiMesh* assimp_mesh)
    {
        StaticLine* line = new_instance<StaticLine>();

        line->aabb(BoxHB(AssimpHelpers::get_vector3(&assimp_mesh->mAABB.mMin),
                         AssimpHelpers::get_vector3(&assimp_mesh->mAABB.mMax)));

        line->mesh(new_instance<StaticLineMesh>());
#define mesh line->_M_mesh

        // Copy vertices
        mesh->data.resize(assimp_mesh->mNumVertices);
        for (unsigned int i = 0; i < assimp_mesh->mNumVertices; i++)
            mesh->data[i].position = AssimpHelpers::get_vector3(&assimp_mesh->mVertices[i]);

        // Copy indeces

        // After triangulate postprocess, each face contains only 3 indeces
        mesh->indexes.reserve(assimp_mesh->mNumFaces * 3);
        for (unsigned int i = 0; i < assimp_mesh->mNumFaces; i++)
        {
            auto& face = assimp_mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) mesh->indexes.push_back(face.mIndices[j]);
        }

        mesh->attributes = {
                {3, BufferValueType::FLOAT, offsetof(StaticLineVertex, position)},
        };

        mesh->mode = DrawMode::STATIC_DRAW;
        mesh->gen();
        mesh->set_data().update_atributes().update_indexes();

        Resources::meshes.push_back(mesh);

#undef mesh
        return line;
    }

    ENGINE_EXPORT void StaticLine::load(const std::string& filename, Scene* scene)
    {
        check(scene);
        const aiScene* assimp_scene = AssimpLibrary::load_scene(filename);


        if (!assimp_scene)
        {
            logger->log("StaticLine: Failed to load '%s'", filename.c_str());
            return;
        }

        struct StackNode {
            SceneTreeNode* scene_parent_node;
            aiNode* node;
        };

        std::stack<StackNode> _M_stack;
        _M_stack.push({scene->scene_head(), assimp_scene->mRootNode});

        while (!_M_stack.empty())
        {
            StackNode node = _M_stack.top();
            _M_stack.pop();
            SceneTreeNode* current_scene_node = Object::new_instance<SceneTreeNode>(node.scene_parent_node);
            current_scene_node->model(AssimpHelpers::get_matrix4(&node.node->mTransformation));
            current_scene_node->name(Strings::to_wstring(node.node->mName.data));

            for (unsigned int i = 0; i < node.node->mNumMeshes; i++)
            {
                auto mesh = assimp_scene->mMeshes[node.node->mMeshes[i]];
                StaticLine* line = load_from_assimp_mesh(mesh);
                scene->push(line, current_scene_node);
            }

            for (unsigned int i = 0; i < node.node->mNumChildren; i++)
                _M_stack.emplace(current_scene_node, node.node->mChildren[i]);
        }

        AssimpLibrary::close_scene(assimp_scene);
    }


}// namespace Engine
