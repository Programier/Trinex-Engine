#include <GL/glew.h>
#include <Graphics/line.hpp>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

namespace Engine
{
    std::size_t sum(const std::vector<int>& vec)
    {
        std::size_t res = 0;
        for (auto& ell : vec) res += ell;
        return res;
    }

    void Line::update()
    {
        Mesh::attributes({3}).vertices_count(_M_data.size() / 3).update_buffers();
    }

    Line::Line() = default;
    Line::Line(const Line& line) = default;
    Line::Line(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes)
        : Mesh(data, vertices, attributes)
    {}

    Line& Line::operator=(const Line& line) = default;

    Line& Line::push_line(const glm::vec3& point1, const glm::vec3& point2)
    {
        _M_data.push_back(point1.x);
        _M_data.push_back(point1.y);
        _M_data.push_back(point1.z);

        _M_data.push_back(point2.x);
        _M_data.push_back(point2.y);
        _M_data.push_back(point2.z);
        update();
        return *this;
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
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(model, aiProcess_Triangulate);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::clog << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return *this;
        }


        std::clog << "Lines loader: Generating meshes" << std::endl;
        // Generating meshes
        auto meshes_count = scene->mNumMeshes;
        for (decltype(meshes_count) i = 0; i < meshes_count; i++)
        {
            auto& scene_mesh = scene->mMeshes[i];
            auto face_count = scene_mesh->mNumFaces;
            for (decltype(face_count) j = 0; j < face_count; j++)
            {
                auto& face = scene_mesh->mFaces[j];
                for (unsigned int f = 0; f < face.mNumIndices; f++)
                {
                    auto& begin = scene_mesh->mVertices[face.mIndices[f]];
                    for (unsigned int g = f + 1; g < face.mNumIndices; g++)
                    {
                        auto& end = scene_mesh->mVertices[face.mIndices[g]];
                        _M_data.push_back(begin.x);
                        _M_data.push_back(begin.y);
                        _M_data.push_back(begin.z);

                        _M_data.push_back(end.x);
                        _M_data.push_back(end.y);
                        _M_data.push_back(end.z);
                    }
                }
            }
        }
        Mesh::attributes({3}).vertices_count(_M_data.size() / 3).update_buffers();
        std::clog << "Lines loader: Loading the \"" << model << "\" model completed successfully" << std::endl;
        return *this;
    }

    Line& Line::lines_from(Model& model)
    {
        auto data = model.meshes();
        std::size_t result_size = 0;
        for (auto& mesh : data) result_size += mesh.vertices_count() * 6;
        _M_data.clear();
        _M_data.reserve(result_size);

        for (auto& mesh : data)
        {
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
        ;
    }

    Line& Line::line_width(const float& width)
    {
        _M_line_width = width;
        return *this;
    }
}// namespace Engine
