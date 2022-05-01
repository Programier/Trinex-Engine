#pragma once
#include <Graphics/mesh.hpp>
#include <Graphics/terrainmodel.hpp>


namespace Engine
{
    class Line : public Mesh, public BasicObject<Translate, Rotate, Scale>
    {
        void update();
        float _M_line_width = 1;

    public:
        Line();
        Line(const Line& line);
        Line(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes);
        Line& operator=(const Line& line);

        Line& push_line(const glm::vec3& point1, const glm::vec3& point2);
        Line& load_from(const std::string& model);
        Line& lines_from(TerrainModel& model);
        float line_width();
        Line& line_width(const float& width);
        Line& draw();
    };

}// namespace Engine
