#pragma once
#include <Graphics/model.hpp>
#include <vector>


namespace Engine
{

    typedef glm::vec3 HeightMapValue;
    typedef HeightMapValue Force;
    typedef std::vector<HeightMapValue> HeightMap_Z_Axis;
    typedef std::vector<HeightMap_Z_Axis> HeightMap_Y_Axis;
    typedef std::vector<HeightMap_Y_Axis> HeightMapArray;
    typedef HeightMapArray HeightMap_X_Axis;


    class HeightMap
    {
    public:
        struct Limit {
            struct {
                float min;
                float max;
            } x, y, z;
        };


    private:
        float _M_block_size;
        Model::Limits _M_limits;
        HeightMapArray _M_array;

    public:
        HeightMap();
        HeightMap(const Model& model, const float& block_size, const glm::mat4& model_matrix = glm::mat4(1.0f));
        HeightMap& from_model(const Model& model, const float& block_size,
                              const glm::mat4& model_matrix = glm::mat4(1.0f));
        const HeightMapArray& array() const;

        std::size_t to_x_index(const float& x_coord) const;
        std::size_t to_y_index(const float& y_coord) const;
        std::size_t to_z_index(const float& z_coord) const;
        const Model::Limits& limits() const;
    };
}// namespace Engine
