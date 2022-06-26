#pragma once
#include <Graphics/mesh.hpp>
#include <Graphics/terrainmodel.hpp>


namespace Engine
{
    class Line : public Mesh, public BasicObject<Translate, Rotate, Scale>
    {
        Line& update();
        Line& _M_push_line(const Point3D& point1, const Point3D& point2);
        Size1D _M_line_width = 1;

    public:
        Line();
        Line(const Line& line);
        Line(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes);
        Line& operator=(const Line& line);

        Line& push_line(const Point3D& point1, const Point3D& point2);
        Line& load_from(const std::string& model);
        Line& lines_from(TerrainModel& model);
        Size1D line_width();
        Line& line_width(const Size1D& width);
        Line& draw();
    };

}// namespace Engine
