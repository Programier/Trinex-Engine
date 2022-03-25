#pragma once

#include <Graphics/mesh.hpp>
#include <Graphics/texture.hpp>
#include <glm/glm.hpp>
#include <list>


namespace Engine
{
    class Model
    {
        struct pair {
            Texture* _M_texture = nullptr;
            Mesh* _M_mesh = nullptr;
            bool empty();
        };

        std::list<Texture> _M_textures;
        std::list<Mesh> _M_meshes;

        std::vector<pair> _M_parts;

        Engine::DrawMode _M_mode;

        void load_textures(const std::vector<std::pair<std::string, const char*>>& names, const bool& invert_textures);

    public:
        Model();
        Model(const std::string& model_file, const DrawMode& mode = Engine::LINEAR, const bool& invert = true);
        Model(const Model&) = delete;
        Model& load_model(const std::string& model_file, const DrawMode& mode = Engine::LINEAR,
                          const bool& invert = true);
        Model& draw();
        const DrawMode& mode();
        Model& mode(const DrawMode& mode);
        ~Model();
    };
}// namespace Engine
