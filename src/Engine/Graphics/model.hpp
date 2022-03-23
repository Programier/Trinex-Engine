#pragma once

#include "mesh.hpp"
#include "texturearray.hpp"
#include <list>


namespace Engine
{
    class Model
    {
        struct ModelMesh {
            Mesh* _M_mesh = nullptr;
            int _M_local_index = 0;
        };

        std::vector<ModelMesh> _M_models;

        std::list<Image> _M_images;
        std::list<Mesh> _M_mesh;
        std::list<unsigned int> _M_texture_array;
        Engine::DrawMode _M_mode;

        void load_textures(const std::vector<std::pair<std::string, const char*>>& names,
                           const bool& invert_textures);

    public:
        Model();
        Model(const std::string& model_file, const DrawMode& mode = Engine::LINEAR,
              const bool& invert = true);
        Model(const Model&) = delete;
        Model& load_model(const std::string& model_file, const DrawMode& mode = Engine::LINEAR,
                          const bool& invert = true);
        Model& draw();
        const DrawMode& mode();
        Model& mode(const DrawMode& mode);
        ~Model();
    };
}// namespace Engine
