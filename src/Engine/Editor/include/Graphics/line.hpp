#pragma once
#include <Graphics/mesh.hpp>
#include <Graphics/basic_object.hpp>


namespace Engine
{
    CLASS Line : public Mesh<float>, public BasicObject<Translate, Rotate, Scale>
    {
        Line& update();
        Line& _M_push_line(const Point3D& point1, const Point3D& point2);

    public:
        Line();
        Line(const Line& line);
        Line(const std::vector<float>& data, unsigned int vertices, const std::vector<MeshAtribute>& attributes);
        Line& operator=(const Line& line);
        Line(Line&&);
        Line& operator = (Line&&);

        Line& push_line(const Point3D& point1, const Point3D& point2);
        Line& load_from(const std::string& model);
        Size1D line_width();
        Line& line_width(const Size1D& width);
        Line& draw();
    };

}// namespace Engine
