#include <Graphics/line.hpp>
#include <LibLoader/lib_loader.hpp>
#include <SDL_log.h>
#include <engine.hpp>
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


    void load_scene(const aiScene* scene, aiNode* node, std::vector<float>& out, glm::mat4 model_matrix = glm::mat4(1.0f))
    {
        SDL_Log("Line loader: Loading %s\n", node->mName.C_Str());

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

    std::size_t sum(const std::vector<int>& vec)
    {
        std::size_t res = 0;
        for (auto& ell : vec) res += ell;
        return res;
    }

    Line& Line::update()
    {
        Mesh::attributes({3}).vertices_count(_M_data.size() / 3).update_buffers();
        return *this;
    }

    Line::Line() = default;
    Line::Line(const Line& line) = default;
    Line::Line(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes)
        : Mesh(data, vertices, attributes)
    {}

    Line& Line::operator=(const Line& line) = default;

    Line& Line::_M_push_line(const glm::vec3& point1, const glm::vec3& point2)
    {
        _M_data.push_back(point1.x);
        _M_data.push_back(point1.y);
        _M_data.push_back(point1.z);

        _M_data.push_back(point2.x);
        _M_data.push_back(point2.y);
        _M_data.push_back(point2.z);
        return *this;
    }

    Line& Line::push_line(const glm::vec3& point1, const glm::vec3& point2)
    {
        return _M_push_line(point1, point2).update();
    }

    Line& Line::draw()
    {
        glLineWidth(_M_line_width);
        Mesh::draw(Engine::LINE);
        glLineWidth(1.0f);
        return *this;
    }

    Line& Line::load_from(const std::string& model)
    {
        _M_data.clear();
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

        SDL_Log("Lines loader: Loading scene\n");
        load_scene(scene, scene->mRootNode, _M_data);
        update();

        SDL_Log("Lines loader: Loading the \"%s\" model completed successfully\n", model.c_str());
        assimp_ReleaseImport(scene);
        return *this;
    }

    Line& Line::lines_from(TerrainModel& model)
    {
        auto data = model.materials();

        std::size_t result_size = 0;
        for (auto& material : data) result_size += material.mesh.vertices_count() * 6;
        _M_data.clear();
        _M_data.reserve(result_size);

        for (auto& material : data)
        {
            auto& mesh = material.mesh;
            auto& vector = mesh.data();
            auto len = vector.size();
            std::size_t attr_sum = sum(mesh.attributes());
            auto block = attr_sum * 3;
            if (len < block)
                continue;
            for (std::size_t i = 0; i < len; i += block)
            {
                // First line
                _M_data.push_back(vector[i]);
                _M_data.push_back(vector[i + 1]);
                _M_data.push_back(vector[i + 2]);

                _M_data.push_back(vector[i + attr_sum]);
                _M_data.push_back(vector[i + 1 + attr_sum]);
                _M_data.push_back(vector[i + 2 + attr_sum]);

                // Second line

                _M_data.push_back(vector[i]);
                _M_data.push_back(vector[i + 1]);
                _M_data.push_back(vector[i + 2]);

                _M_data.push_back(vector[i + (2 * attr_sum)]);
                _M_data.push_back(vector[i + 1 + (2 * attr_sum)]);
                _M_data.push_back(vector[i + 2 + (2 * attr_sum)]);

                // Third line
                _M_data.push_back(vector[i + attr_sum]);
                _M_data.push_back(vector[i + 1 + attr_sum]);
                _M_data.push_back(vector[i + 2 + attr_sum]);

                _M_data.push_back(vector[i + (2 * attr_sum)]);
                _M_data.push_back(vector[i + 1 + (2 * attr_sum)]);
                _M_data.push_back(vector[i + 2 + (2 * attr_sum)]);
            }
        }
        Mesh::attributes({3}).vertices_count(_M_data.size() / 3).update_buffers();
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