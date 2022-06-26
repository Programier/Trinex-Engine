#pragma once

#include <Graphics/basic_object.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/texture.hpp>
#include <glm/glm.hpp>
#include <unordered_map>


namespace Engine
{
    class TerrainModel : public BasicObject<Translate, Rotate, Scale>
    {
    public:
        typedef SizeLimits3D Limits;

        struct Material {
            std::size_t index = 0;
            bool render = true;
            std::string name;
            ReferenceWrapper<Texture> texture;
            Mesh mesh;
        };

        struct TerrainTexture {
            Texture texture;
            bool default_texture = false;
        };

    private:
        std::unordered_map<std::string, TerrainTexture> _M_textures;
        Color _M_default_color;
        TerrainModel::Limits _M_limits;
        Engine::DrawMode _M_mode;
        std::vector<Material> _M_materials;


    public:
        TerrainModel();
        TerrainModel(const std::string& model_file, const DrawMode& mode = Engine::LINEAR, const MipMapLevel& mipmap_level = 4,
                     const bool& invert = true);
        TerrainModel(const TerrainModel&) = delete;
        TerrainModel& load_model(const std::string& model_file, const DrawMode& mode = Engine::LINEAR,
                                 const MipMapLevel& mipmap_level = 4, const bool& invert = true);
        TerrainModel& draw();
        const DrawMode& mode();
        TerrainModel& mode(const DrawMode& mode);
        const std::vector<Material>& materials() const;
        const TerrainModel::Limits& limits() const;
        Color default_color() const;
        TerrainModel& default_color(const Color& color);
        bool& material_render_status(const std::size_t& material_index);
        ~TerrainModel();
    };
}// namespace Engine
