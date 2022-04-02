#pragma once
#include <Graphics/terrainmodel.hpp>
#include <vector>


namespace Engine
{

    typedef std::size_t ArrayIndex;
    typedef glm::vec3 Force;

    struct HeightMapValue {
        ArrayIndex x = 0;
        ArrayIndex y = 0;
        ArrayIndex z = 0;
        glm::vec3 position = {0.f, 0.f, 0.f};
        Force normal = {0.f, 0.f, 0.f};

        bool operator<(const HeightMapValue&) const;
        bool operator<=(const HeightMapValue&) const;
        bool operator>(const HeightMapValue&) const;
        bool operator>=(const HeightMapValue&) const;
        bool operator==(const HeightMapValue&) const;
        bool operator!=(const HeightMapValue&) const;
    };

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
        TerrainModel::Limits _M_limits;
        HeightMapArray _M_array;

    public:
        HeightMap();
        HeightMap(const TerrainModel& model, const float& block_size, const glm::mat4& model_matrix = glm::mat4(1.0f));
        HeightMap& from_model(const TerrainModel& model, const float& block_size,
                              const glm::mat4& model_matrix = glm::mat4(1.0f));
        const HeightMapArray& array() const;

        ArrayIndex to_x_index(const float& x_coord) const;
        ArrayIndex to_y_index(const float& y_coord) const;
        ArrayIndex to_z_index(const float& z_coord) const;
        const TerrainModel::Limits& limits() const;
        float block_size() const;
    };
}// namespace Engine
