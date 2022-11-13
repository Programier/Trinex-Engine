#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/line.hpp>
#include <Graphics/scene.hpp>
#include <Graphics/shader_system.hpp>
#include <LibLoader/lib_loader.hpp>
#include <api_funcs.hpp>
#include <glm/glm.hpp>
#include <model_loader.hpp>
#include <opengl.hpp>

namespace Engine
{
    static glm::mat4 mat4(const aiMatrix4x4& matrix)
    {
        glm::mat4 result;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) result[i][j] = matrix[i][j];
        return result;
    }

    static glm::vec3 vec3(const aiVector3D& vector)
    {
        return glm::vec3(vector.x, vector.y, vector.z);
    }


    static void load_scene(const aiScene* scene, aiNode* node, std::vector<float>& out,
                           glm::mat4 model_matrix = glm::mat4(1.0f))
    {
        logger->log("Line loader: Loading %s\n", node->mName.C_Str());
        logger->log("MESHES: %u\n", node->mNumMeshes);

        model_matrix = mat4(node->mTransformation.Transpose()) * model_matrix;
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            auto& mesh = scene->mMeshes[node->mMeshes[i]];
            for (unsigned int f = 0; f < mesh->mNumFaces; f++)
            {
                auto& face = mesh->mFaces[f];
                for (unsigned int index = 0; index < face.mNumIndices; index++)
                {
                    auto begin = vec3(mesh->mVertices[face.mIndices[index]]);
                    begin = model_matrix * glm::vec4(begin, 1.f);
                    for (unsigned int g = index + 1; g < face.mNumIndices; g++)
                    {
                        auto end = vec3(mesh->mVertices[face.mIndices[g]]);
                        end = model_matrix * glm::vec4(end, 1.f);
                        out.push_back(begin.x);
                        out.push_back(begin.y);
                        out.push_back(begin.z);

                        out.push_back(end.x);
                        out.push_back(end.y);
                        out.push_back(end.z);
                    }
                }
            }
        }


        for (unsigned int i = 0; i < node->mNumChildren; i++) load_scene(scene, node->mChildren[i], out, model_matrix);
    }

    Line& Line::update()
    {
        attributes = {{3, BufferValueType::FLOAT}};
        vertices = data.size() / 3;
        if (Mesh::id() == 0)
        {
            Mesh::mode = DrawMode::STATIC_DRAW;
            Mesh::gen();
        }
        set_data().update_atributes();
        return *this;
    }

    Line::Line() = default;
    Line::Line(const Line& line) = default;
    Line::Line(const std::vector<float>& data, unsigned int vertices, const std::vector<MeshAtribute>& attributes)
    {
        this->data = data;
        this->vertices = vertices;
        this->attributes = attributes;
    }

    Line& Line::operator=(const Line& line) = default;

    Line& Line::_M_push_line(const glm::vec3& point1, const glm::vec3& point2)
    {
        data.push_back(point1.x);
        data.push_back(point1.y);
        data.push_back(point1.z);

        data.push_back(point2.x);
        data.push_back(point2.y);
        data.push_back(point2.z);
        return *this;
    }

    Line& Line::push_line(const glm::vec3& point1, const glm::vec3& point2)
    {
        return _M_push_line(point1, point2).update();
    }

    DrawableObject* Line::copy() const
    {
        throw not_implemented;
    }

    void Line::render_layer(const glm::mat4& prev_model)
    {
        if (!_M_visible)
            return;

        Scene* scene = Scene::get_active_scene();
        if (!scene)
        {
            throw std::runtime_error("No active scene found!");
        }

        Scene::ActiveCamera& active_camera = scene->active_camera();

        if (!active_camera.camera)
        {
            throw std::runtime_error("No active camera found!");
        }

        namespace shd = ShaderSystem::Line;

        auto model = prev_model * this->_M_model.get();
        shd::shader.use().set(shd::model, model).set(shd::color, color).set(shd::projview, active_camera.projview);

        float tmp = get_current_line_rendering_width();
        set_line_rendering_width(_M_line_width);
        BasicMesh::draw(Engine::Primitive::LINE);
        set_line_rendering_width(tmp);
    }

    Line& Line::load_from(const std::string& model)
    {
        data.clear();
        Library assimp = load_library("assimp");
        if (!assimp.has_lib())
            return *this;

        auto assimp_ReleaseImport = assimp.get<void, const C_STRUCT aiScene*>(lib_function(aiReleaseImport));

        const aiScene* scene = load_scene(model);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            assimp_ReleaseImport(scene);
            return *this;
        }

        logger->log("Lines loader: Loading scene\n");
        load_scene(scene, scene->mRootNode, data);
        update();

        logger->log("Lines loader: Loading the \"%s\" model completed successfully\n", model.c_str());
        assimp_ReleaseImport(scene);
        return *this;
    }


    float Line::line_width()
    {
        return _M_line_width;
    }

    Line& Line::line_width(const float& width)
    {
        _M_line_width = width;
        return *this;
    }

    Line::Line(Line&& line) = default;
    Line& Line::operator=(Line&& line) = default;
}// namespace Engine
