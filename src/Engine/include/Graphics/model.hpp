#pragma once

#include <Graphics/mesh.hpp>
#include <Graphics/texture.hpp>
#include <glm/glm.hpp>
#include <list>


namespace Engine
{
    class Model
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

        Model::Limits _M_limits;

    public:
        Model();
        Model(const std::string& model_file, const DrawMode& mode = Engine::LINEAR,
              const unsigned int& mipmap_level = 4, const bool& invert = true);
        Model(const Model&) = delete;
        Model& load_model(const std::string& model_file, const DrawMode& mode = Engine::LINEAR,
                          const unsigned int& mipmap_level = 4, const bool& invert = true);
        Model& draw();
        const DrawMode& mode();
        Model& mode(const DrawMode& mode);

        const std::list<Texture>& textures() const;
        const std::list<Mesh>& meshes() const;
        const std::vector<pair>& parts() const;
        const Model::Limits& limits() const;
        ~Model();
    };
}// namespace Engine
