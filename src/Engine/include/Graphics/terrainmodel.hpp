#pragma once

#include <Graphics/basic_object.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/texture.hpp>
#include <glm/glm.hpp>


namespace Engine
{
    class TerrainModel : public BasicObject<TranslateObject, RotateObject, ScaleObject>
    {
    public:
        struct Limits {
            glm::vec3 min;
            glm::vec3 max;
        };

        struct pair {
            Texture* _M_texture = nullptr;
            Mesh* _M_mesh = nullptr;
            bool empty();
        };

    private:
        std::list<Texture> _M_textures;
        std::list<Mesh> _M_meshes;

        std::vector<pair> _M_parts;

        Engine::DrawMode _M_mode;

        void load_textures(const std::vector<std::pair<std::string, const char*>>& names, const unsigned int& mipmap,
                           const bool& invert_textures);

        TerrainModel::Limits _M_limits;

    public:
        TerrainModel();
        TerrainModel(const std::string& model_file, const DrawMode& mode = Engine::LINEAR, const unsigned int& mipmap_level = 4,
                     const bool& invert = true);
        TerrainModel(const TerrainModel&) = delete;
        TerrainModel& load_model(const std::string& model_file, const DrawMode& mode = Engine::LINEAR,
                                 const unsigned int& mipmap_level = 4, const bool& invert = true);
        TerrainModel& draw();
        const DrawMode& mode();
        TerrainModel& mode(const DrawMode& mode);

        const std::list<Texture>& textures() const;
        const std::list<Mesh>& meshes() const;
        const std::vector<pair>& parts() const;
        const TerrainModel::Limits& limits() const;
        ~TerrainModel();
    };
}// namespace Engine
