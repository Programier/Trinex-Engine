#pragma once
#include <Core/color.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/drawable_object.hpp>
#include <Graphics/mesh.hpp>


namespace Engine
{
    CLASS Line : public Mesh<float>, public DrawableObject
    {
        Line& _M_push_line(const Point3D& point1, const Point3D& point2);
        float _M_line_width = 1.f;

    public:
        Color color = Color::White;

        Line();
        Line(const Line& line);
        Line(const std::vector<float>& data, unsigned int vertices, const std::vector<MeshAtribute>& attributes);
        Line& operator=(const Line& line);
        Line(Line &&);
        Line& operator=(Line&&);

        Line& push_line(const Point3D& point1, const Point3D& point2);
        Line& load_from(const std::string& model);
        Size1D line_width();
        Line& line_width(const Size1D& width);
        Line& update();

        DrawableObject* copy() const override;
        bool is_empty_layer() const override;
        void render_layer(const glm::mat4& prev_model = Constants::identity_matrix,
                          on_render_layer_func = empty_drawable_callback_handler) const override;
    };

}// namespace Engine
